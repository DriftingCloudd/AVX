#include "include/thread.h"
#include "include/memlayout.h"
#include "include/kalloc.h"
#include "include/vm.h"
#include "include/proc.h"
#include "include/futex.h"

int nexttid = 1;

thread threads[THREAD_NUM];
thread *free_thread;

void threadInit() {
    for (int i = 0; i < THREAD_NUM; i++) {
        threads[i].state = t_UNUSED;
        threads[i].pre_thread = NULL;
        threads[i].next_thread = NULL;
        if (i != 0) {
            threads[i].pre_thread = &threads[i - 1];
        }
        if (i + 1 < THREAD_NUM) {
            threads[i].next_thread = &threads[i + 1];
        }
    }
    free_thread = &threads[0];
}
// alloc main thread
thread *allocNewThread() {
    /*
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
    threads[i].awakeTime = 0;
    threads[i].state = t_RUNNABLE;
    threads[i].tid = nexttid++;
    
    
    return &threads[i];
    */
    if (NULL == free_thread) {
        panic("allocNewThread: can not find unused thread");
    }
    if (NULL == (free_thread->trapframe = (struct trapframe *)kalloc())) {
        panic("allocNewThread: can not kalloc a page");
    }
    free_thread->awakeTime = 0;
    free_thread->state = t_RUNNABLE;
    free_thread->tid = nexttid++;
    free_thread->xstate = 0;
    free_thread->clear_child_tid = 0;

    // 取出链表头部
    if (NULL != free_thread->next_thread)  // 重要！链表操作要判断元素非空
        free_thread->next_thread->pre_thread = NULL;
    thread *tmp = free_thread;
    free_thread = free_thread->next_thread;
    tmp->next_thread = NULL;
    tmp->pre_thread = NULL;

    return tmp;
}

void thread_free(thread *t) {
    struct proc *p = t->p;
    if (t->clear_child_tid) {
        int val = 0;
        copyout(p->pagetable,t->clear_child_tid,(char *)&val,sizeof(int));
        futexWake(t->clear_child_tid,1); // TODO: check
    }
    // kfree掉内存
    kfree((void*)t->trapframe);
    if (t->kstack != p->kstack)
      kfree((void*)t->kstack_pa);
    // 更改链表情况
    t->state = t_UNUSED;
    if (NULL != t->pre_thread)
        t->pre_thread->next_thread = t->next_thread;
    if (NULL != t->next_thread)
        t->next_thread->pre_thread = t->pre_thread;
    if (NULL != free_thread)
        free_thread->pre_thread = t;
    t->next_thread = free_thread;
    free_thread = t;
    // 查看进程情况
    p->thread_num--;
    if (!p->thread_num) {
        p->xstate = t->xstate;
        exitproc(p);
    }
}

void thread_destroy(thread *t) {
    thread_free(t);
    if (t->p->state == ZOMBIE) {   // 已经摧毁了进程
        exit_sched();
        panic("zombie exit");
    } else {
        exit_yield();
    }
}
