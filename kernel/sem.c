#include "include/sem.h"
#include "include/spinlock.h"
#include "include/proc.h"
#include "include/types.h"
#include "include/timer.h"

void sem_init(struct semaphore *sem, uint8 value) {
    sem->value = value;
    sem->valid = 1;
    initlock(&sem->lock, "semaphore");
}

void sem_wait(struct semaphore *sem) {
    acquire(&sem->lock);
    while (sem->value <= 0) {
        sleep(sem, &sem->lock);
    }
    sem->value--;
    release(&sem->lock);
}

uint32 sem_wait_with_milli_timeout(struct semaphore *sem, time_t milli_timeout) {
    time_t begin, end;
    begin = get_timeval().tv_usec;
    end = begin + milli_timeout * 1000;
    acquire(&sem->lock);
    while (sem->value <= 0) {
        end = get_timeval().tv_usec;
        if (milli_timeout * 1000 <= end - begin) {
            release(&sem->lock);
            return milli_timeout;
        }
        sleep(sem, &sem->lock);
    }
    sem->value--;
    release(&sem->lock);
    return end - begin;
}

void sem_post(struct semaphore *sem) {
    acquire(&sem->lock);
    sem->value++;
    wakeup(sem);
    release(&sem->lock);
}

void sem_destroy(struct semaphore *sem) {
    acquire(&sem->lock);
    sem->value = 0;
    sem->valid = 0;
    release(&sem->lock);
}

void sem_set_invalid(struct semaphore *sem) {
    acquire(&sem->lock);
    sem->valid = 0;
    release(&sem->lock);
}

int sem_is_valid(struct semaphore *sem) {
    int valid;
    acquire(&sem->lock);
    valid = sem->valid;
    release(&sem->lock);
    return valid;
}


