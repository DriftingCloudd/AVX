#include "include/proc.h"
#include "include/futex.h"
#include "include/timer.h"
#include "include/thread_group.h"
#include "include/types.h"
#include "include/printf.h"
#include "include/string.h"

uint64 futex_sleep(wq_t *futex_queue, void *chan, timespec_t *time) {
    uint64 res = 0;
    proc_t *p = myproc();
    p->futex_chan = chan;
    if (time) {
        int timeout = time->tv_sec*1000 + time->tv_nsec/1000000;
        res = wait_event_timeout_interruptible_locked(futex_queue,p->futex_chan == NULL, timeout);
    } else {
        res = wait_event_interruptible_locked(futex_queue,p->futex_chan == NULL);
    }

    return res;
}

int futex_wake(wq_t *futex_queue, void *chan, int n, int requeue, void *newaddr, int requeue_lim) {
    proc_t *p;
    wq_entry_t *entry, *tmp;
    int wakeup_num = 0;
    list_for_each_entry_safe_condition(entry, tmp, &futex_queue->head, head, wakeup_num < n) {
        p = entry->private_p;
        if(p->futex_chan == chan) {
            p->futex_chan = NULL;
            entry->func(futex_queue, entry);
            wakeup_num++;
            debug("PID %d woken", p->pid);
        }
    }
    if(requeue) {
        int requeue_num = 0;
        list_for_each_entry_condition(entry, &futex_queue->head, head, requeue_num < requeue_lim) {
            p = entry->private_p;
            if(p->futex_chan == chan) {
                if(p->futex_chan == chan) {
                    p->futex_chan = newaddr;
                    requeue_num++;
                    debug("PID %d requeued to %#lx", p->pid, newaddr);
                }
            }
        }
    }
    return wakeup_num;
}