
#include "include/types.h"
#include "include/param.h"
#include "include/memlayout.h"
#include "include/riscv.h"
#include "include/spinlock.h"
#include "include/sleeplock.h"
#include "include/proc.h"
#include "include/elf.h"
#include "include/fat32.h"
#include "include/kalloc.h"
#include "include/vm.h"
#include "include/printf.h"
#include "include/string.h"

#define STACK_SIZE 36*PGSIZE
enum redir{
  REDIR_OUT,
  REDIR_APPEND,
};
extern uint64 open(char* path, int omode);

// va must be page-aligned
// and the pages from va to va+sz must already be mapped.
// Returns 0 on success, -1 on failure.
static int
loadseg(pagetable_t pagetable, uint64 va, struct dirent *ep, uint offset, uint sz)
{
  uint i = 0, n;
  uint64 pa;
  uint64 va_off = 0;
  if((va % PGSIZE) != 0){
    va_off = va % PGSIZE;
    va = va - va_off;
    pa = walkaddr(pagetable, va);
    if(pa == NULL)
      panic("loadseg: address should exist");
    n = PGSIZE - va_off;
    if(eread(ep, 0, (uint64)(pa + va_off), offset, n) != n){
      printf("loadseg: read error\n");
      return -1;
    }
    i += n;
    va += PGSIZE;
  }

  for( ; i < sz; ){
    pa = walkaddr(pagetable, va);
    if(pa == NULL)
      panic("loadseg: address should exist");
    if(sz - i < PGSIZE)
      n = sz - i;
    else
      n = PGSIZE;
    if(eread(ep, 0, (uint64)pa, offset+i, n) != n)
      return -1;
    i += n;
    va += PGSIZE;
  }

  return 0;
}

//read and check elf header
//return 0 on success,-1 on failure
static int
readelfhdr(struct dirent* ep,struct elfhdr* elf){
  // Check ELF header
  if(eread(ep, 0, (uint64)elf, 0, sizeof(struct elfhdr)) != sizeof(struct elfhdr))
    return -1;
  if(elf->magic != ELF_MAGIC)
    return -1;
  return 0;
}

pagetable_t create_kpagetable(struct proc *p){
  pagetable_t kpagetable;
  // Make a copy of p->kpt without old user space, 
  // but with the same kstack we are using now, which can't be changed
  if ((kpagetable = (pagetable_t)kalloc()) == NULL) {
    return 0;
  }
  memmove(kpagetable, p->kpagetable, PGSIZE);
  for (int i = 0; i < PX(2, MAXUVA); i++) {
    kpagetable[i] = 0;
  }
  return kpagetable;
}

void
stackdisplay(pagetable_t pagetable,uint64 sp,uint64 sz)
{
  for(uint64 i = sp;i<sz;i+=8){
    uint64 *pa = (void*)walkaddr(pagetable,i) + i - PGROUNDDOWN(i);
    if(pa)printf("addr %p phaddr:%p value %p\n",i, pa, *pa);
    else printf("addr %p value (nil)\n",i);
  }
}

//加载elf文件，成功返回0，失败返回-1
uint64
loadelf(struct elfhdr* elf, struct dirent* ep, struct proghdr* phdr,pagetable_t pagetable, pagetable_t kpagetable,uint64 * sz){
  uint64 sz1;
  int i,off;
  struct proghdr ph;
  int getphdr = 0;
  // Load program into memory.
  for(i=0, off=elf->phoff; i<elf->phnum; i++, off+=sizeof(struct proghdr)){
    if(eread(ep, 0, (uint64)&ph, off, sizeof(struct proghdr)) != sizeof(struct proghdr)){
      printf("eread failed\n");
      return -1;
    }
    if(ph.type == ELF_PROG_LOAD){
      if(ph.memsz < ph.filesz){
        printf("ph.memsz < ph.filesz\n");
        return -1;
      }
      if(ph.vaddr + ph.memsz < ph.vaddr){
        printf("ph.vaddr + ph.memsz < ph.vaddr\n");
        return -1;
      }
      if(!getphdr&&phdr&&ph.off == 0){ 
        phdr->vaddr = elf->phoff + ph.vaddr;
      }
      int perm = 0;
      if(ph.flags & ELF_PROG_FLAG_EXEC)
        perm |= PTE_X;
      if(ph.flags & ELF_PROG_FLAG_WRITE)
        perm |= PTE_W;
      if(ph.flags & ELF_PROG_FLAG_READ)
        perm |= PTE_R;
      if((sz1 = uvmalloc(pagetable, kpagetable, *sz, PGROUNDUP(ph.vaddr + ph.memsz), perm)) == 0){
        printf("uvmalloc failed\n");
        return -1;
      }
      *sz = sz1;
      if(loadseg(pagetable, ph.vaddr, ep, ph.off, ph.filesz) < 0){
        printf("loadseg failed\n");
        return -1;
      }
    }else if(ph.type == ELF_PROG_PHDR){
      if(phdr){
        getphdr = 1;
        *phdr = ph;
      }
    }else{
      //printf("ph.type: %d ph.type != ELF_PROG_LOAD\n", ph.type);
    }
  }
  // debug_print("loadelf success\n");
  return 0;
}

int
create_user_stack(uint64 * sz, uint64 * sp, uint64 * stackbase, pagetable_t pagetable, pagetable_t kpagetable){

  *sz = PGROUNDUP(*sz);
  uint64 sz1;
  if((sz1 = uvmalloc(pagetable, kpagetable, *sz, *sz + STACK_SIZE, PTE_R | PTE_W)) == 0){
    return -1;
  }
  *sz = sz1;
  uvmclear(pagetable, *sz - STACK_SIZE);
  *sp = *sz;
  *stackbase = *sp - STACK_SIZE + PGSIZE;
  return 0;
}

int
user_stack_push_str(pagetable_t pt, uint64 * ustack, char * str, uint64 sp, uint64 stackbase){
  uint64 argc = ++ustack[0];
  if(argc>MAXARG+1){
    return -1;
  }
  sp -= strlen(str) + 1;
  sp -= sp % 16; // riscv sp must be 16-byte aligned
  if(sp < stackbase){
    return -1;
  }
  if(copyout(pt, sp, str, strlen(str) + 1) < 0){
    printf("copyout failed\n");
    return -1;
  }
  ustack[argc] = sp;
  ustack[argc+1] = 0;
  return sp;
}

void
alloc_aux(uint64* aux,uint64 atid,uint64 value)
{
  //printf("aux[%d] = %p\n",atid,value);
  uint64 argc = aux[0];
  aux[argc*2+1] = atid;
  aux[argc*2+2] = value;
  aux[argc*2+3] = 0;
  aux[argc*2+4] = 0;
  aux[0]++;
}

int
loadaux(pagetable_t pagetable,uint64 sp,uint64 stackbase,uint64* aux){
  int argc = aux[0];
  if(!argc)return sp;
  /*
  printf("aux argc:%d\n",argc);
  for(int i=1;i<=2*argc+2;i++){
    printf("final raw aux[%d] = %p\n",i,aux[i]);
  }
  */
  sp -= (2*argc+2) * sizeof(uint64);
  if(sp < stackbase){
    return -1;
  }
  aux[0] = 0;
  if(copyout(pagetable, sp, (char *)(aux+1), (2*argc+2)*sizeof(uint64)) < 0){
    return -1;
  }
  return sp;
}

int exec(char *path, char **argv, char ** env)
{
  char *s, *last;
  uint64 argc, sz = 0, sp, ustack[MAXARG+1], stackbase;
  struct elfhdr elf;
  struct dirent *ep;
  struct proghdr ph;
  pagetable_t pagetable = 0, oldpagetable;
  pagetable_t kpagetable = 0, oldkpagetable;
  struct proc *p = myproc();

  kpagetable = create_kpagetable(p);
  if(kpagetable == 0){
    debug_print("[exec]create_kpagetable failed\n");
    return -1;
  }
  // printf("coming 1\n");
  if((ep = ename(path)) == NULL) {
    #ifdef DEBUG
    printf("[exec] %s not found\n", path);
    #endif
    goto bad;
  }
  elock(ep);

  if(readelfhdr(ep, &elf) < 0){
    printf("readelfhdr failed\n");
    goto bad;
  }

  if((pagetable = proc_pagetable(p)) == NULL)
    goto bad;

  // Load program into memory.
  if(loadelf(&elf, ep, &ph, pagetable, kpagetable, &sz) < 0){
    printf("loadelf failed\n");
    goto bad;
  }
  eunlock(ep);
  eput(ep);
  ep = 0;

  p = myproc();
  uint64 oldsz = p->sz;
  if(create_user_stack(&sz, &sp, &stackbase, pagetable, kpagetable) == -1){
    printf("create_user_stack failed\n");
    goto bad;
  }
  //printf("[exec] stackbase: %p stacktop: %p\n", stackbase, sz);

  //下面开始处理environment的问题
  uint64 envp[MAXARG+1];
  envp[0] = 0;
  if((sp = user_stack_push_str(pagetable, envp, "LD_LIBRARY_PATH=/", sp, stackbase)) == -1){
    printf("user_stack_push_str failed 1\n");
    goto bad;
  }
  // if((sp = user_stack_push_str(pagetable, envp, "PATH=/", sp, stackbase)) == -1){
  //   printf("user_stack_push_str failed 2\n");
  //   goto bad;
  // }

  //添加随机数
  uint64 random[2] = { 0xcde142a16cb93072, 0x128a39c127d8bbf2 };
  sp -= 16;
  if (sp < stackbase || copyout(pagetable, sp, (char *)random, 16) < 0) {
    printf("[exec] random copy bad\n");
    goto bad;
  }

  uint64 aux[MAXARG*2+3] = {0,0,0};
  alloc_aux(aux,AT_PAGESZ,PGSIZE);
  alloc_aux(aux,AT_PHDR, ph.vaddr);
  alloc_aux(aux,AT_PHENT, elf.phentsize);
  alloc_aux(aux,AT_PHNUM, elf.phnum);
  alloc_aux(aux,AT_UID, 0);
  alloc_aux(aux,AT_EUID, 0);
  alloc_aux(aux,AT_GID, 0);
  alloc_aux(aux,AT_EGID, 0);
  alloc_aux(aux,AT_SECURE, 0);
  alloc_aux(aux,AT_EGID, 0);		
  alloc_aux(aux,AT_RANDOM, sp);
  // printf("coming 2\n");
  // Push argument strings, prepare rest of stack in ustack.
  ustack[0] = 0;
  int redirection = -1;
  int jump = 0;
  char * redir_file = 0;
  for(argc = 0; argv[argc]; argc++) {
    if(strlen(argv[argc]) == 1 && strncmp(argv[argc], ">", 1) == 0){
      // printf("redirection 1 %s\n", argv[argc]);
      redirection = REDIR_OUT;
      continue;
    }else if(strlen(argv[argc]) == 2 && strncmp(argv[argc], ">>", 2) == 0){
      // printf("redirection 2\n");
      redirection = REDIR_APPEND;
      continue;
    }
    if(redirection != -1 && jump == 0){
      redir_file = argv[argc];
      jump = 1;
      continue;
    }
    if(argc >= MAXARG)
      goto bad;
    if((sp = user_stack_push_str(pagetable, ustack, argv[argc], sp, stackbase)) == -1){
      printf("user_stack_push_str failed 2\n");
      goto bad;
    }
  }
  uint64 envnum = 0;
  if(env){
    for(envnum = 0; env[envnum]; envnum++){
      if((sp = user_stack_push_str(pagetable, envp, env[envnum], sp, stackbase)) == -1){
        printf("user_stack_push_str failed 3\n");
        goto bad;
      }
    }
  }

  if((sp = loadaux(pagetable,sp,stackbase,aux)) == -1){
    printf("loadaux failed\n");
    goto bad;
  }

  argc = envp[0];
  if(argc){
    sp -= (argc+1) * sizeof(uint64);
    if(sp < stackbase){
      printf("sp < stackbase\n");
      goto bad;
    }
    sp -= sp%16;
    if(copyout(pagetable, sp, (char *)(envp+1), (argc+1)*sizeof(uint64)) < 0){
      printf("copyout failed\n");
      goto bad;
    }
  }
  argc = ustack[0];
  // push the array of argv[] pointers.
  sp -= (argc+2) * sizeof(uint64);
  sp -= sp % 16;
  if(sp < stackbase)
    goto bad;
  if(copyout(pagetable, sp, (char *)ustack, (argc+2)*sizeof(uint64)) < 0)
    goto bad;

  // printf("ustack[0]:%d\n",ustack[0]);
  // stackdisplay(pagetable, sp, sz);
  // arguments to user main(argc, argv)
  // argc is returned via the system call return
  // value, which goes in a0.
  p->trapframe->a1 = sp + 8;

  // Save program name for debugging.
  for(last=s=path; *s; s++)
    if(*s == '/')
      last = s+1;
  safestrcpy(p->name, last, sizeof(p->name));
    
  // Commit to the user image.
  oldpagetable = p->pagetable;
  oldkpagetable = p->kpagetable;
  p->pagetable = pagetable;
  p->kpagetable = kpagetable;
  p->sz = sz;
  p->trapframe->epc = elf.entry;  // initial program counter = main
  p->trapframe->sp = sp; // initial stack pointer
  
  // maybe it's wrong
  for (int fd = 0; fd < NOFILEMAX(p); fd++) {
    struct file *f = p -> ofile[fd];
    if (f && p->exec_close[fd]) {
      fileclose(f);
      p->ofile[fd] = 0;
      p->exec_close[fd] = 0;
    }
  }
  //处理重定向
  if(redirection != -1){
    fileclose(p->ofile[1]);
    p->ofile[1] = 0;
    int fd = 0;
    if(redirection == REDIR_OUT){
      fd = open(redir_file, O_WRONLY | O_CREATE);
    }else if(redirection == REDIR_APPEND){
      fd = open(redir_file, O_WRONLY | O_CREATE | O_APPEND);
    }
    if(fd != 1){
      debug_print("[exec]:fd != 1\n");
      goto bad;
    }
    p->exec_close[1] = 1;
  }

  proc_freepagetable(oldpagetable, oldsz);
  w_satp(MAKE_SATP(p->kpagetable));
  sfence_vma();
  kvmfree(oldkpagetable, 0);
  
  return argc; // this ends up in a0, the first argument to main(argc, argv)

 bad:
  #ifdef DEBUG
  printf("[exec] reach bad\n");
  #endif
  if(pagetable)
    proc_freepagetable(pagetable, sz);
  if(kpagetable)
    kvmfree(kpagetable, 0);
  if(ep){
    eunlock(ep);
    eput(ep);
  }
  return -1;
}
