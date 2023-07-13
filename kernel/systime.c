#include "include/types.h"
#include "include/param.h"
#include "include/riscv.h"
#include "include/syscall.h"
#include "include/file.h"
#include "include/timer.h"
#include "include/vm.h"
#include "include/proc.h"

//todo
// int utimensat(int dirfd, const char *pathname,
//                     const struct timespec times[_Nullable 2], int flags);
uint64 sys_utimensat(void)
{
    int fd;
	struct proc *p = myproc();
    struct file *fp = NULL;
    struct file *f;
    uint64 pathAddr, times;
	TimeSpec t[2];
    char pathName[255];
	struct dirent *dp, ep;
    int flags, devNo;
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
	
	dp = fp->ep;
	// ep = ename(dp, pathName, &devNo);

}