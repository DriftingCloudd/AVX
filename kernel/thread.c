#include "include/thread.h"

Thread threads[THREAD_TOTAL_NUMBER];  // 暂时先定义100个线程？

struct ThreadList freeThreads,usedThreads;
struct ThreadList scheduleList[2];
Thread *currentThread[5] = {0};  // 有4个核心

struct spinlock threadListLock,scheduleListLock,threadIdLock;

Thread *myThread() {
    if (NULL == currentThread[r_mhartid()]) 
        return NULL;
    
    return currentThread[r_mhartid()];
}

