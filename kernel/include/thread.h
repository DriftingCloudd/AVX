#ifndef _Thread_H_
#define _Thread_H_

#include "types.h"
#include "fat32.h"
#include "types.h"
#include "queue.h"
#include "riscv.h"

#define THREAD_NUM 2510

enum threadState {
    t_UNUSED,
    t_SLEEPING,
    t_RUNNABLE,
    t_RUNNING,
    t_ZOMBIE
};

typedef struct t_context {
    uint64 ra;
    uint64 sp;

    // callee-saved
    uint64 s0;
    uint64 s1;
    uint64 s2;
    uint64 s3;
    uint64 s4;
    uint64 s5;
    uint64 s6;
    uint64 s7;
    uint64 s8;
    uint64 s9;
    uint64 s10;
    uint64 s11;
}t_context;

typedef struct thread {
    struct spinlock lock;

    // 当使用下面这些变量的时候，thread的锁必须持有
    enum threadState state;   // 线程的状态
    struct proc *p;    // 这个线程属于哪一个进程
    void *chan;   // 如果不为NULL,则在chan地址上睡眠
    int tid;   // 线程ID

    // 使用下面这些变量的时候，thread的锁不需要持有
    uint64 kstack;   // 线程内核栈的地址,一个进程的不同线程所用的内核栈的地址应该不同
    uint64 sz;   // 复制自进程的sz
    struct trapframe *trapframe;
    t_context context;  // 每个进程应该有自己的context
    uint64 clear_child_tid;
    // TODO: signal
}thread;

void threadInit();
thread *allocNewThread();

#endif