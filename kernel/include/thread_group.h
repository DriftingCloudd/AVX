#ifndef _H_THREAD_GROUP_
#define _H_THREAD_GROUP_

#include "types.h"
#include "spinlock.h"
#include "list.h"
#include "waitqueue.h"

struct thread_group {
    int tg_id;
    int ref;
    int thrdcnt;
    wq_t futex_wq;
    spinlock_t lock;
    list_head_t member;
};

typedef struct thread_group tg_t;

/**
 * @brief create a new thread group
 * 
 * @param p main process whose pid will be used as PID intsead of TID
 * @return tg_t* 
 */
tg_t *tg_new(proc_t *p);

/**
 * @brief free a thread group
 * 
 * @param pself 
 */
void tg_free(tg_t **pself);

void tg_ref(tg_t *t);
void tg_join(tg_t *t, proc_t *p);
int tg_thrd_cnt(tg_t *t);
int tg_quit(tg_t *t);

/**
 * @brief get PID
 * 
 * @param self 
 * @return int PID of this thread group
 */
int tg_pid(tg_t *t);

#endif