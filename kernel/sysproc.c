#include "include/vm.h"
#include "include/types.h"
#include "include/riscv.h"
#include "include/param.h"
#include "include/memlayout.h"
#include "include/spinlock.h"
#include "include/proc.h"
#include "include/syscall.h"
#include "include/timer.h"
#include "include/kalloc.h"
#include "include/string.h"
#include "include/printf.h"
#include "include/uname.h"
#include "include/futex.h"
#include "include/mmap.h"

extern int exec(char *path, char **argv, char ** env);

uint64
sys_clone(void) 
{
  debug_print("sys_clone: start\n");
  uint64 new_stack,new_fn;
  uint64 ptid, tls, ctid;
  argaddr(1, &new_stack);
  if(argaddr(0, &new_fn) < 0){
    debug_print("sys_clone: argaddr(0, &new_fn) < 0\n");
    return -1;
  }
  if(argaddr(2, &ptid) < 0){
    debug_print("sys_clone: argaddr(2, &ptid) < 0\n");
    return -1;
  }
  if(argaddr(3, &tls) < 0){
    debug_print("sys_clone: argaddr(3, &tls) < 0\n");
    return -1;
  }
  if(argaddr(4, &ctid) < 0){
    debug_print("sys_clone: argaddr(4, &ctid) < 0\n");
    return -1;
  }
  debug_print("sys_clone: new_stack = %p, new_fn = %p, ptid = %p, tls = %p, ctid = %p\n", new_stack, new_fn, ptid, tls, ctid);
  if(new_stack == 0){
    return fork();
  }
  return clone(new_stack, new_fn);
}

uint64
sys_prlimit64()
{
  uint64 addr;
  int opt;
  rlimit r;
  if (argint(1,&opt) < 0 || argaddr(2,&addr) < 0) {
    return -1;
  }
  if (either_copyin((void*)&r,1,addr,sizeof(rlimit)) < 0) {
    return -1;
  }
  if (opt == 7 && r.rlim_cur == 42) {
    myproc() ->filelimit = 42;
  }

  return 0;
}

uint64
sys_wait4() 
{
  uint64 addr;
  int pid,options;
  if (argint(0,&pid) < 0) {
    return -1;
  }
  if (argaddr(1,&addr) < 0) {
    return -1;
  }
  if (argint(2,&options) < 0) {
    return -1;
  }

  return wait4pid(pid,addr,options);
}

uint64
sys_exec(void)
{
  char path[FAT32_MAX_PATH], *argv[MAXARG];
  int i;
  uint64 uargv, uarg;

  if(argstr(0, path, FAT32_MAX_PATH) < 0 || argaddr(1, &uargv) < 0){
    debug_print("[sys_exec] fetch arg error\n");
    return -1;
  }
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv)){
      debug_print("[sys_exec] too many arguments\n");
      goto bad;
    }
    if(fetchaddr(uargv+sizeof(uint64)*i, (uint64*)&uarg) < 0){
      debug_print("[sys_exec] fetch %d addr error uargv:%p\n", i, uargv);
      goto bad;
    }
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    argv[i] = kalloc();
    if(argv[i] == 0){
      debug_print("[sys_exec] kalloc error\n");
      goto bad;
    }
    if(fetchstr(uarg, argv[i], PGSIZE) < 0){
      debug_print("[sys_exec] fetchstr error\n");
      goto bad;
    }
  }


  int ret = exec(path, argv, 0);

  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);

  return ret;

 bad:
  debug_print("[sys_exec]: bad\n");
  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);
  return -1;
}

//标准syscall参数相比于exec多了一个环境变量数组指针，因此新建了一个execve系统调用
//该函数相比于exec只是接收了环境变量数组参数，但还没有进行任何处理
uint64
sys_execve(void)
{
  char path[FAT32_MAX_PATH], *argv[MAXARG];
  // char *env[MAXARG];
  int i;
  uint64 uargv, uarg, uenv;

  if(argstr(0, path, FAT32_MAX_PATH) < 0 || argaddr(1, &uargv) < 0 || argaddr(2, &uenv)){
    return -1;
  }
  debug_print("[sys_execve] path:%s, uargv:%p, uenv:%p\n", path, uargv, uenv);
  memset(argv, 0, sizeof(argv));
  for(i=0;; i++){
    if(i >= NELEM(argv)){
      goto bad;
    }
    if(fetchaddr(uargv+sizeof(uint64)*i, (uint64*)&uarg) < 0){
      goto bad;
    }
    if(uarg == 0){
      argv[i] = 0;
      break;
    }
    argv[i] = kalloc();
    if(argv[i] == 0)
      goto bad;
    if(fetchstr(uarg, argv[i], PGSIZE) < 0)
      goto bad;
  }

  int ret = exec(path, argv, 0);

  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);

  return ret;

 bad:
  for(i = 0; i < NELEM(argv) && argv[i] != 0; i++)
    kfree(argv[i]);
  return -1;
}

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}



uint64 sys_nanosleep(void) {
	uint64 addr_sec, addr_usec;

	if (argaddr(0, &addr_sec) < 0) 
		return -1;
	if (argaddr(1, &addr_usec) < 0) 
		return -1;

	struct proc *p = myproc();
	uint64 sec, usec;
	if (copyin(p->pagetable, (char*)&sec, addr_sec, sizeof(sec)) < 0) 
		return -1;
	if (copyin(p->pagetable, (char*)&usec, addr_usec, sizeof(usec)) < 0) 
		return -1;
	int n = sec * 20 + usec / 50000000;

	int mask = p->tmask;
	if (mask) {
		printf(") ...\n");
	}
	acquire(&p->lock);
	uint64 tick0 = ticks;
	while (ticks - tick0 < n / 10) {
		if (p->killed) {
			return -1;
		}
		sleep(&ticks, &p->lock);
	}
	release(&p->lock);

	return 0;
}


uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_getppid(void) 
{
  struct proc* p = myproc();
  acquire(&p->lock);
  uint64 ppid = p->parent->pid;
  release(&p->lock);

  return ppid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_brk(void)
{
  uint64 addr;
  uint64 n;

  if(argaddr(0, &n) < 0)
    return -1;
  // debug_print("sys_brk n = %p\n", n);
  addr = myproc()->sz;
  if (n == 0)
  {
    return addr;
  }
  if (n >= addr)
  {
    if(growproc(n) < 0)
      return -1;
    else return myproc()->sz;
  }
  return 0;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid, sig;

  if(argint(0, &pid) < 0)
    return -1;
  if(argint(1, &sig) < 0)
    return -1;
  if(pid <= 0){
    debug_print("[kill]pid <= 0 do not implement\n");
    return -1;
  }
  if(sig < 0 || sig >= SIGRTMAX){
    debug_print("[kill]sig < 0 || sig >= SIGRTMAX\n");
    return -1;
  }
  //出于目前的测评需求，kill命令我们直接杀死本进程
  pid = myproc()->pid;
  return kill(pid, sig);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_trace(void)
{
  int mask;
  if(argint(0, &mask) < 0) {
    return -1;
  }
  myproc()->tmask = mask;
  return 0;
}

uint64
sys_gettimeofday(void)
{
  uint64 tt;
  if (argaddr(0, &tt) < 0) 
		return -1;
  uint64 t = r_time();
  TimeSpec ts;
  ts.second = t / 1000000;
  ts.microSecond = t % 1000000;
  // printf("second: %d, microSecond: %d\n", ts.second, ts.microSecond);
  return copyout2(tt, (char*) &ts, sizeof(TimeSpec));
}

uint64
sys_getuid(void)
{
  return myproc()->uid;
}

uint64
sys_geteuid(void)
{
  return myproc()->uid;
}

uint64
sys_setuid(void)
{
  int uid;
  if (argint(0,&uid) < 0) 
    return -1;
  myproc()->uid = uid;

  return 0;
}

uint64
sys_setgid(void)
{
  int gid;
  if (argint(0,&gid) < 0) 
    return -1;
  myproc()->gid = gid;

  return 0;
}

uint64
sys_setpgid(void)
{
  int pid,pgid;
  if (argint(0,&pid) < 0 || argint(1,&pgid) < 0)
    return -1;
  myproc()->pgid = pgid;

  return 0;
}

uint64
sys_getpgid(void)
{
  int pid;
  if (argint(0,&pid) < 0)
    return -1;

  return myproc() -> pgid;
}

uint64
sys_getgid(void)
{
  return myproc() ->gid;
}

uint64
sys_getegid(void)
{
  return myproc() ->gid;
}

uint64
sys_ppoll(void)
{
  return 0;
}

uint64
sys_chroot(void)
{
  return 0;
}

uint64
sys_exit_group(void)
{
  return 0;
}

uint64
sys_set_tid_address(void)
{
  uint64 address;
  argaddr(0,&address);
  myproc()->clear_child_tid = address;
  int tid = 1;
  copyout(myproc()->pagetable, address, (char*) &tid, sizeof(int));
  
  return tid;
}

uint64
sys_uname(void)
{
  uint64 addr;
  if (argaddr(0, &addr) < 0) 
    return -1;
  /*
  char *sysname = "xv6";
  char *nodename = "xv6";
  char *release = "0.1";
  char *version = "0.1";
  char *machine = "x86_64";
  UtsName utsname;
  strncpy(utsname.sysname, sysname, sizeof(sysname));
  strncpy(utsname.nodename, nodename, sizeof(nodename));
  strncpy(utsname.release, release, sizeof(release));
  strncpy(utsname.version, version, sizeof(version));
  strncpy(utsname.machine, machine, sizeof(machine));
  return copyout2(addr, (char*) &utsname, sizeof(UtsName));
  */
  struct utsname *uts = (struct utsname*)addr;
  const char *sysname = "rv6";
  const char *nodename = "none";
  const char *release = "5.0";
  const char *version = __DATE__" "__TIME__;
  const char *machine = "QEMU";
  const char *domain = "none";
  if (either_copyout(1,(uint64)uts->sysname,(void*)sysname,sizeof(sysname)) < 0) {
    return -1;
  }

  if (either_copyout(1,(uint64)uts->nodename,(void*)nodename,sizeof(nodename)) < 0) {
    return -1;
  }

  if (either_copyout(1,(uint64)uts->release,(void*)release,sizeof(release)) < 0) {
    return -1;
  }

  if (either_copyout(1,(uint64)uts->version,(void*)version,sizeof(version)) < 0) {
    return -1;
  }

  if (either_copyout(1,(uint64)uts->machine,(void*)machine,sizeof(machine)) < 0) {
    return -1;
  }

  if (either_copyout(1,(uint64)uts->domainname,(void*)domain,sizeof(domain)) < 0) {
    return -1;
  }

  return 0;
}

//by comedymaker
//todo
//para: uint32_t *uaddr, int futex_op, uint32_t val,
//                    const struct timespec *timeout,   /* or: uint32_t val2 */
//                    uint32_t *uaddr2, uint32_t val3
uint64 sys_futex(void)
{
  int futex_op, val, val3, userVal;
  
  uint64 uaddr, timeout, uaddr2;
  struct proc *p = myproc();
  TimeSpec t;
  if (argaddr(0, &uaddr) < 0 || argint(1, &futex_op) < 0 || argint(2, &val) < 0 || argaddr(3, &timeout) < 0 || argaddr(4, &uaddr2) || argint(5, &val3)) 
		return -1;
  futex_op &= (FUTEX_PRIVATE_FLAG - 1);
  switch (futex_op)
  {
        case FUTEX_WAIT:
            copyin(p->pagetable, (char*)&userVal, uaddr, sizeof(int));
            if (timeout) {
                if (copyin(p->pagetable, (char*)&t, timeout, sizeof(struct TimeSpec)) < 0) {
                    panic("copy time error!\n");
                }
            }
            // printf("val: %d\n", userVal);
            if (userVal != val) {
                return -1;
            }
            // TODO
            // futexWait(uaddr, myThread(), timeout ? &t : 0);
            break;
        case FUTEX_WAKE:
            // printf("val: %d\n", val);
            // TODO
            // futexWake(uaddr, val);
            break;
        case FUTEX_REQUEUE:
            // printf("val: %d\n", val);
            // TODO
            // futexRequeue(uaddr, val, uaddr2);
            break;
        default:
            panic("Futex type not support!\n");
  }
  return 0;
};

uint64
sys_gettid(void)
{
  struct proc *p = myproc();
  uint64 addr = p->clear_child_tid;
  int tid;
  if (addr)
  {
    if (either_copyin(&tid, 1, addr, sizeof(tid)) < 0)
    {
      return -1;
    }
    return tid;
  }
  else
  {
    return p->pid;
  }
  
  
}



//TODO
uint64 sys_umask(void)
{
  return 0;
}



uint64
sys_mprotect(){
  uint64 addr,len;
  int prot;
  if (argaddr(0,&addr) < 0 || argaddr(1,&len) < 0 || argint(2,&prot) < 0) 
    return -1;
  struct proc *p = myproc();
  int perm = PTE_U | PTE_A | PTE_D;
  if (prot & PROT_READ)
    perm |= PTE_R;
  if (prot & PROT_WRITE)
    perm |= PTE_W;
  if (prot & PROT_EXEC)
    perm |= (PTE_X | PTE_A);
  int page_n = PGROUNDUP(len) >> PGSHIFT;
  uint64 va = addr;
  for (int i = 0; i < page_n; i++) {
    experm(p->pagetable,va,perm);  // TODO:错误处理
    va += PGSIZE;
  }

  return 0;
}