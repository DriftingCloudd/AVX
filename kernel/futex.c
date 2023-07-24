#include "include/futex.h"
#include "include/proc.h"
#include "include/types.h"
#include "include/timer.h"
#include "include/thread.h"

typedef struct FutexQueue
{
    uint64 addr;
    Thread* thread;
    uint8 valid;
} FutexQueue;

FutexQueue futexQueue[FUTEX_COUNT];

void futexWait(uint64 addr, Thread* th, TimeSpec2* ts) {
    for (int i = 0; i < FUTEX_COUNT; i++) {
        if (!futexQueue[i].valid) {
            futexQueue[i].valid = 1;
            futexQueue[i].addr = addr;
            futexQueue[i].thread = th;
            if (ts) {
                th->awakeTime = ts->tv_sec * 1000000 + ts->tv_nsec / 1000;
            } else {
                th->state = SLEEPING;
            }
            yield();
        }
    }
    panic("No futex Resource!\n");
}

void futexWake(uint64 addr, int n) {
    for (int i = 0; i < FUTEX_COUNT && n; i++) {
        if (futexQueue[i].valid && futexQueue[i].addr == addr) {
            futexQueue[i].thread->state = RUNNABLE;
            futexQueue[i].thread->trapframe.a0 = 0; // set next yield accept!
            futexQueue[i].valid = 0;
            n--;
        }
    }
}

void futexRequeue(uint64 addr, int n, uint64 newAddr) {
    for (int i = 0; i < FUTEX_COUNT && n; i++) {
        if (futexQueue[i].valid && futexQueue[i].addr == addr) {
            futexQueue[i].thread->state = RUNNABLE;
            futexQueue[i].thread->trapframe.a0 = 0; // set next yield accept!
            futexQueue[i].valid = 0;
            n--;
        }
    }
    for (int i = 0; i < FUTEX_COUNT; i++) {
        if (futexQueue[i].valid && futexQueue[i].addr == addr) {
            futexQueue[i].addr = newAddr;
        }
    }
}

void futexClear(Thread* thread) {
    for (int i = 0; i < FUTEX_COUNT; i++) {
        if (futexQueue[i].valid && futexQueue[i].thread == thread) {
            futexQueue[i].valid = 0;
        }
    }
}