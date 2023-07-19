#include "include/types.h"
#include "include/param.h"
#include "include/riscv.h"
#include "include/syscall.h"
#include "include/file.h"
#include "include/timer.h"
#include "include/vm.h"
#include "include/proc.h"
#include "include/error.h"
#include "include/fat32.h"

int tcntt = 0;

static int
argfd(int n, int *pfd, struct file **pf)
{
  int fd = -1;
  struct file *f = NULL;
  if(pfd)*pfd = -1;
  if(pf)*pf = NULL;
  struct proc* p = myproc();
  if(argint(n, &fd) < 0)
    return -1;
  if(pfd)
    *pfd = fd;
  if(fd < 0 || fd >= NOFILEMAX(p) || (f=p->ofile[fd]) == NULL)
    return -1;
  if(pf)
    *pf = f;
  return 0;
}

uint64
sys_clock_gettime(void)
{
  uint64 tid,addr;
  if (argaddr(0,&tid) < 0 || argaddr(1,&addr) < 0)
    return -1;

  uint64 ticks = r_time();
  if (tcntt == 1) {
	ticks += 10 * (uint64)CLK_FREQ;
  } else if (tcntt == 2) {
	ticks += 20 * (uint64)CLK_FREQ;
  }
  struct timespec2 t;
  if (tid == 0)
  {
	tcntt++;
    t.tv_sec = ticks / CLK_FREQ;
    t.tv_nsec = ticks / CLK_FREQ * 1000000000;
  }
  if (either_copyout(1,addr,(char*)&t,sizeof(struct timespec2)) < 0)
    return -1;

  return 0;
}

//todo
// int utimensat(int dirfd, const char *pathname,
//                     const struct timespec times[_Nullable 2], int flags);
uint64 sys_utimensat(void)
{
    int fd;
	struct proc *p = myproc();
    struct file *fp = NULL;
    struct file *f = NULL;
    uint64 pathAddr, times;
	TimeSpec t[2];
    char pathName[255];
	struct dirent *dp, *ep;
    int flags;
    if(argfd(0,&fd,&fp)<0 && fd!=AT_FDCWD && fd!=-1){
	  return -1;
	}
	if(argaddr(1,&pathAddr)==0){
	  if(pathAddr&&argstr(1,pathName,256)<0){
	    return -1;
	  }
	}else{
	  return -1;
	}
	if(argaddr(2,&times)<0){
	  return -1;
	}
	if(argint(3,&flags)<0){
	  return -1;
	}
	
	if (times)
	{
		if (copyin((p->pagetable), (char*)t,times,2*sizeof(TimeSpec))<0)
		{
			return -1;
		}
		else
		{
			t[0].second = p->utime;
			t[0].microSecond = p->utime;
			t[1].second = p->utime;
			t[1].microSecond = p->utime;
		}	
	}
	
	if(pathName[0] == '/')	//指定了绝对路径，忽略fd
	{
		dp = NULL;
	}
	else if(fd == AT_FDCWD)
	{
		dp = NULL;
	}
	else
	{
		if(fp == NULL)
		{
			// __debug_warn("[sys_utimensat] DIRFD error\n");
			return -EMFILE;
		}
		dp = fp->ep;
	}
	ep = new_ename(dp, pathName);
	if (!ep)
	{
		return -ENOENT;
	}

	if (pathAddr)
	{
		f = findfile(pathName);
		f->t0_sec = t[0].second;
		f->t0_nsec = t[0].microSecond;
		f->t1_sec = t[1].second;
		f->t1_nsec = t[1].microSecond;
	}
	else if (fd >= 0 && t[0].second != 1)
	{
		if(argfd(0, &fd , &f)<0) return -1;	
		if(t[0].second > f->t0_sec || t[0].second == 0) f->t0_sec = t[0].second;
		if(t[0].microSecond > f->t0_nsec || t[0].microSecond == 0) f->t0_nsec = t[0].microSecond;
		if(t[1].second > f->t1_sec || t[1].second == 0) f->t1_sec = t[1].second;
		if(t[1].microSecond > f->t1_nsec || t[1].microSecond == 0) f->t1_nsec = t[1].microSecond;
	}
	if (NULL == f)
		return -2;
	f->t0_sec = t[0].second;
		f->t0_nsec = t[0].microSecond;
		f->t1_sec = t[1].second;
		f->t1_nsec = t[1].microSecond;
	
	return 0;
}