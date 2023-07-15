#ifndef __FILE_H
#define __FILE_H

#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x040
#define O_TRUNC   0x200
#define O_APPEND  0x400
#define O_DIRECTORY 0x010000
#define O_CLOEXEC 0x80000
#define AT_FDCWD  -100

struct file {
  enum { FD_NONE, FD_PIPE, FD_ENTRY, FD_DEVICE } type;
  int ref; // reference count
  char readable;
  char writable;
  struct pipe *pipe; // FD_PIPE
  struct dirent *ep;
  uint off;          // FD_ENTRY
  short major;       // FD_DEVICE
  uint64 t0_sec;
  uint64 t0_nsec;
  uint64 t1_sec;
  uint64 t1_nsec;
};

// #define major(dev)  ((dev) >> 16 & 0xFFFF)
// #define minor(dev)  ((dev) & 0xFFFF)
// #define	mkdev(m,n)  ((uint)((m)<<16| (n)))

// map major device number to device functions.
struct devsw {
  int (*read)(int, uint64, int);
  int (*write)(int, uint64, int);
};

typedef struct {
  void  *iov_base;    /* Starting address */
  uint32 iov_len;     /* Number of bytes to transfer */
}iovec;
#define IOVMAX 64

extern struct devsw devsw[];

#define CONSOLE 1
#define SEEK_SET  (int)0
#define SEEK_CUR  (int)1
#define SEEK_END  (int)2


struct file*    filealloc(void);
void            fileclose(struct file*);
struct file*    filedup(struct file*);
void            fileinit(void);
int             fileread(struct file*, uint64, int n);
int             filestat(struct file*, uint64 addr);
int             filewrite(struct file*, uint64, int n);
int             dirnext(struct file *f, uint64 addr);
int             get_next_dirent(struct file *f, uint64 addr, int n);
uint64          fileseek(struct file *f, uint64 offset, int whence);

struct file*    findfile(char* path);
#endif