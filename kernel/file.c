//
// Support functions for system calls that involve file descriptors.
//


#include "include/types.h"
#include "include/riscv.h"
#include "include/param.h"
#include "include/spinlock.h"
#include "include/sleeplock.h"
#include "include/fat32.h"
#include "include/file.h"
#include "include/pipe.h"
#include "include/stat.h"
#include "include/proc.h"
#include "include/printf.h"
#include "include/string.h"
#include "include/vm.h"

struct devsw devsw[NDEV];
struct {
  struct spinlock lock;
  struct file file[NFILE];
} ftable;

void
fileinit(void)
{
  initlock(&ftable.lock, "ftable");
  struct file *f;
  for(f = ftable.file; f < ftable.file + NFILE; f++){
    memset(f, 0, sizeof(struct file));
  }
  #ifdef DEBUG
  printf("fileinit\n");
  #endif
}

// Allocate a file structure.
struct file*
filealloc(void)
{
  struct file *f;

  acquire(&ftable.lock);
  for(f = ftable.file; f < ftable.file + NFILE; f++){
    if(f->ref == 0){
      f->ref = 1;
      release(&ftable.lock);
      return f;
    }
  }
  release(&ftable.lock);
  return NULL;
}

// Increment ref count for file f.
struct file*
filedup(struct file *f)
{
  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("filedup");
  f->ref++;
  release(&ftable.lock);
  return f;
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
  struct file ff;

  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("fileclose");
  if(--f->ref > 0){
    release(&ftable.lock);
    return;
  }
  ff = *f;
  f->ref = 0;
  f->type = FD_NONE;
  release(&ftable.lock);

  if(ff.type == FD_PIPE){
    pipeclose(ff.pipe, ff.writable);
  } else if(ff.type == FD_ENTRY){
    eput(ff.ep);
  } else if (ff.type == FD_DEVICE) {

  }
}

// Get metadata about file f.
// addr is a user virtual address, pointing to a struct stat.
int
filestat(struct file *f, uint64 addr)
{
  // struct proc *p = myproc();
  struct kstat kst;
  
  if(f->type == FD_ENTRY){
    elock(f->ep);
    kstat(f->ep, &kst);
    eunlock(f->ep);
    // if(copyout(p->pagetable, addr, (char *)&st, sizeof(st)) < 0)
    if(copyout2(addr, (char *)&kst, sizeof(kst)) < 0)
      return -1;
    return 0;
  }
  return -1;
}

uint64
fileinput(struct file* f, int user, uint64 addr, int n, uint64 off){
  uint64 r = 0;
  switch (f->type) {
    case FD_PIPE:
        r = piperead(f->pipe, user, addr, n);
        break;
    case FD_DEVICE:
        r = (devsw + f->major)->read(user, addr, n);
        break;
    case FD_ENTRY:
        r = eread(f->ep, user, addr, off, n);
        break;
    case FD_NONE:
    	return 0;
  }
  return r;
}

uint64
fileoutput(struct file* f, int user, uint64 addr, int n, uint64 off){
  uint64 r = 0;
  switch (f->type) {
    case FD_PIPE:
        r = pipewrite(f->pipe, user, addr, n);
        break;
    case FD_DEVICE:
        r = (devsw + f->major)->write(user, addr, n);
        break;
    case FD_ENTRY:
        r = ewrite(f->ep, user, addr, off, n);
        break;
    case FD_NONE:
    	return 0;
  }
  return r;
}

// Read from file f.
// addr is a user virtual address.
int
fileread(struct file *f, uint64 addr, int n)
{
  int r = 0;

  if(f->readable == 0)
    return -1;

  switch (f->type) {
    case FD_PIPE:
        r = piperead(f->pipe, 1, addr, n);
        break;
    case FD_DEVICE:
        if(f->major < 0 || f->major >= NDEV || !devsw[f->major].read)
          return -1;
        r = devsw[f->major].read(1, addr, n);
        break;
    case FD_ENTRY:
        elock(f->ep);
          if((r = eread(f->ep, 1, addr, f->off, n)) > 0)
            f->off += r;
        eunlock(f->ep);
        break;
    default:
      panic("fileread");
  }

  return r;
}

// Write to file f.
// addr is a user virtual address.
int
filewrite(struct file *f, uint64 addr, int n)
{
  int ret = 0;

  if(f->writable == 0)
    return -1;

  if(f->type == FD_PIPE){
    ret = pipewrite(f->pipe, 1, addr, n);
  } else if(f->type == FD_DEVICE){
    if(f->major < 0 || f->major >= NDEV || !devsw[f->major].write)
      return -1;
    ret = devsw[f->major].write(1, addr, n);
  } else if(f->type == FD_ENTRY){
    elock(f->ep);
    if (ewrite(f->ep, 1, addr, f->off, n) == n) {
      ret = n;
      f->off += n;
    } else {
      ret = -1;
    }
    eunlock(f->ep);
  } else {
    panic("filewrite");
  }

  return ret;
}

// Read from dir f.
// addr is a user virtual address.
int
dirnext(struct file *f, uint64 addr)
{
  // struct proc *p = myproc();

  if(f->readable == 0 || !(f->ep->attribute & ATTR_DIRECTORY))
    return -1;

  struct dirent de;
  struct stat st;
  int count = 0;
  int ret;
  elock(f->ep);
  while ((ret = enext(f->ep, &de, f->off, &count)) == 0) {  // skip empty entry
    f->off += count * 32;
  }
  eunlock(f->ep);
  if (ret == -1)
    return 0;

  f->off += count * 32;
  estat(&de, &st);
  // if(copyout(p->pagetable, addr, (char *)&st, sizeof(st)) < 0)
  if(copyout2(addr, (char *)&st, sizeof(st)) < 0)
    return -1;

  return 1;
}

struct file*
findfile(char* path)
{
  char name[FAT32_MAX_FILENAME + 1];
  // struct dirent* ep = ename(NULL,path,&dev);
  struct dirent* ep = new_lookup_path(NULL ,path, 0, name);
  struct proc* p = myproc();
  if(ep == NULL)return NULL;
  elock(ep);
  for(int i = 0;i<NOFILEMAX(p);i++){
    if(p->ofile[i]->type==FD_ENTRY&&p->ofile[i]->ep==ep){
      eunlock(ep);
      eput(ep);
      return p->ofile[i];
    }
    if(p->ofile[i]->type==FD_DEVICE){
      eunlock(ep);
      eput(ep);
      return p->ofile[i];
    }
  }
  eunlock(ep);
  eput(ep);
  return NULL;
}

// 和dirnext不同，要新建一个类linux的目录结构
int
get_next_dirent(struct file *f, uint64 addr, int n)
{
  struct proc *p = myproc();
  if(f->readable == 0 || !(f->ep->attribute & ATTR_DIRECTORY))
    return -1;
  struct dirent de;
  struct dirent64 lde;
  int ret = 0,cnt = 0,copysize = 0;
  elock(f->ep);  // 锁住条目
  while (1) {
    lde.d_off = f->off;
    ret = enext(f->ep,&de,f->off,&cnt);
    f->off += (cnt << 5);
    // 如果当前的条目是一个空的
    if (0 == ret) {
      continue;
    }
    // 如果已经到了文件末尾了
    if (-1 == ret) {
      eunlock(f->ep);
      return copysize;
    }

    memcpy(lde.d_name, de.filename, sizeof(de.filename));
    lde.d_ino = 0;
    lde.d_type = (de.attribute & ATTR_DIRECTORY) ? T_DIR : T_FILE;

    // 计算大小，需要考虑内存对齐和可变长文件名
    int size = sizeof(struct dirent64) - sizeof(lde.d_name) + strlen(lde.d_name) + 1;  // +1是因为字符串最后要保存一个0
    size += (sizeof(uint64) - (size % sizeof(uint64))) % sizeof(uint64);
    lde.d_reclen = size;

    // 如果已经复制完n个字节了，就直接退出
    if (lde.d_reclen > n) {
      break;
    }
    
    int cpsz = lde.d_reclen;

    if (copyout(p->pagetable,addr,(char*)&lde,cpsz) < 0) {
      eunlock(f->ep);
      return -1;
    }

    addr += cpsz;
    n -= cpsz;
    copysize += cpsz;

  }
  eunlock(f->ep);

  f->off += (cnt << 5);

  return copysize;
}

uint64
fileseek(struct file *f, uint64 offset, int whence)
{
  uint64 ret = -1;
  if (f->type == FD_ENTRY) {
    elock(f->ep);
    if (whence == SEEK_SET) {
        ret = offset;
        f ->off = offset;
    } else if (whence == SEEK_CUR) {
        f ->off += offset;
        ret = f ->off;
    } else if (whence == SEEK_END) {
      f ->off = f ->ep ->file_size + offset;
      ret = f ->off;
    }
    eunlock(f->ep);
  } else if (f->type == FD_PIPE) {
    acquire(&f->pipe->lock);
    if (whence == SEEK_SET) {
      f ->off = offset;
      ret = offset;
    } else if (whence == SEEK_CUR) {
      f ->off += offset;
      ret = f ->off;
    }
    release(&f->pipe->lock);
  }

  return ret;
}

int fileillegal(struct file* f){
  switch (f->type) {
    case FD_PIPE:
    case FD_DEVICE:
        // if(f->major < 0 || f->major >= getdevnum() || !devsw[f->major].read || !devsw[f->major].write)
        //   return 1;
        if(f->major < 0 || !devsw[f->major].read || !devsw[f->major].write)
          return 1;
    case FD_ENTRY:
        break;
    default:
      panic("fileillegal");
      return 1;
  }
  return 0;
}

void fileiolock(struct file* f){
  switch (f->type) {
    case FD_PIPE:
        acquire(&f->pipe->lock);
        break;
    case FD_DEVICE:
        // acquire(&(devsw + f->major)->lk);
        break;
    case FD_ENTRY:
        elock(f->ep);
        break;
    case FD_NONE:
    	return;
  }
}

void fileiounlock(struct file* f){
  switch (f->type) {
    case FD_PIPE:
        release(&f->pipe->lock);
        break;
    case FD_DEVICE:
        // release(&(devsw + f->major)->lk);
        break;
    case FD_ENTRY:
        eunlock(f->ep);
        break;
    case FD_NONE:
    	return;
  }
}

//TODO
uint64 file_send(struct file* fin,struct file* fout,uint64 addr,uint64 n)
{
  uint64 off = 0;
  uint64 rlen = 0;
  uint64 wlen = 0;
  uint64 ret = 0;
  if(addr){
    if(either_copyin(&off,1,addr,sizeof(uint64))<0){
      return -1;
    }
  }else{
    off = fin->off;
  }
  if(fileillegal(fin)||fileillegal(fout)){
      return -1;
  }
  fileiolock(fin);
  fileiolock(fout);
  while(n){
    char buf[512];
    // rlen = MIN(n,512);
    if (n > 512)
    {
      rlen = 512;
    }
    else
    {
      rlen = n;
    }
    
    
    rlen = fileinput(fin,0,(uint64)&buf,rlen,off);
    printf("[filesend] send rlen %p\n",rlen);
    off += rlen;
    n -= rlen;
    if(!rlen){
      break;
    }
    wlen = fileoutput(fout,0,(uint64)&buf,rlen,fout->off);
    printf("[filesend] send wlen:%p\n",rlen,wlen);
    fout->off += wlen;
    ret += wlen;
  }
  fileiounlock(fout);
  fileiounlock(fin);
  //printf("[filesend]after send fout off:%p\n",fout->off);
  if(addr){
    if(either_copyout(1,addr,&off,sizeof(uint64))<0){
      return -1;
    }
  }else{
    fin->off = off;
  }
  return ret;
  
}