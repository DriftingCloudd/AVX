#include "include/thread.h"
#include "include/memlayout.h"
#include "include/kalloc.h"
#include "include/vm.h"

int nexttid = 1;

thread threads[THREAD_NUM];

void threadInit() {
    for (int i = 0; i < THREAD_NUM; i++) {
        threads[i].state = t_UNUSED;
    }
}

thread *allocNewThread() {
    int i = 0;
    for (i = 0; i < THREAD_NUM; i++) {
        if (threads[i].state == t_UNUSED)
            break;
    }
    if (t_UNUSED != threads[i].state) {
        panic("allocNewThread: can not find unused thread");
    }
    if (NULL == (threads[i].trapframe = (struct trapframe *)kalloc())) {
        panic("allocNewThread: can not kalloc a page");
    }
    threads[i].state = t_RUNNABLE;
    threads[i].tid = nexttid++;
    
    
    return &threads[i];
}

/*
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

extern pagetable_t kernel_pagetable;

void threadInit() {
    initlock(&threadListLock,"threadList");
    initlock(&scheduleListLock,"scheduleList");
    initlock(&threadIdLock,"threadId");

    LIST_INIT(&freeThreads);
    LIST_INIT(&usedThreads);
    LIST_INIT(&scheduleList[0]);
    LIST_INIT(&scheduleList[1]);

    for (int i = THREAD_TOTAL_NUMBER - 1; i >= 0; i--) {
        threads[i].state = UNUSED;
        threads[i].trapframe.kernelSatp = MAKE_SATP(kernel_pagetable);
        LIST_INSERT_HEAD(&freeThreads,&threads[i],link);
    }
}

void wakeup_thread(void *chan) {
    Thread *thread = NULL;
    LIST_FOREACH(thread,&usedThreads,link) {
        if (thread->state == SLEEPING && thread->chan == (uint64)chan) 
            thread->state = RUNNABLE;
    }
}

uint32 getThreadId(Thread *t) {
    static uint32 pid = 0;
    uint32 threadId = (++pid << (1 + LOG_PROCESS_NUM)) | (uint32)(t - threads);

    return threadId;
}

int tid2Thread(uint32 threadId, struct Thread **thread, int checkPerm) {
    struct Thread *t;
    int hardId = r_mhartid();

    if (0 == threadId) {
        *thread = currentThread[hardId];
        return 0;
    }

    t = threads + PROCESS_OFFSET(threadId);

    if (t->state == UNUSED || t->id != threadId) {
        *thread = NULL;
        return -1;
    }

    if (checkPerm) {
        if (t != currentThread[hardId])   { // TODO: check parentId 
            *thread = NULL;
            return -1;
        }
    }

    *thread = t;
    return 0;
}

uint64 getThreadTopSp(Thread *t) {
    return KERNEL_PROCESS_SP_TOP - (uint64)(t - threads) * 10 * PGSIZE;
}

void threadSetup(Thread *t) {
    t->chan = 0;
    t->retValue = 0;
    t->state = UNUSED;
    t->reason = 0;
    t->setChildTid = t->clearChildTid = 0;
    t->awakeTime = 0;
    t->robustHeadPointer = 0;
    t->killed = 0;
    void *pa = NULL;
    for (int i = 1; i <= 3; i++) {
        if (NULL == (pa = kalloc())) {
            panic("threadSetup: kalloc");
        }
        if (mappages(kernel_pagetable,getThreadTopSp(t) - PGSIZE * i,PGSIZE,(uint64)pa,PTE_R | PTE_W) < 0) {
            panic("threadSetup: mappages");
        }
    }
}
*/