#include "include/types.h"
#include "include/param.h"
#include "include/riscv.h"
#include "include/syscall.h"
#include "include/file.h"
#include "include/timer.h"

//todo
// int utimensat(int dirfd, const char *pathname,
//                     const struct timespec times[_Nullable 2], int flags);
uint64 sys_utimensat(void)
{
    int fd;
    struct file *fp = NULL;
    struct file *f;
    uint64 path_addr;
    char path_name[255];
    int flags;
    if(argfd(0,&fd,&fp)<0 && fd!=AT_FDCWD && fd!=-1){
	  return -1;
	}
	if(argaddr(1,&path_addr)==0){
	  if(path_addr&&argstr(1,path_name,256)<0){
	    return -1;
	  }
	}else{
	  return -1;
	}
	if(argaddr(2,&buf)<0){
	  return -1;
	}
	if(argint(3,&flags)<0){
	  return -1;
	}
}