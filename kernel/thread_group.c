#include "include/thread_group.h"
#include "include/proc.h"
#include "include/spinlock.h"
#include "include/printf.h"
#include "include/kalloc.h"

void tg_ref(tg_t *t) {
    acquire(&t->lock);
    t->ref++;
    release(&t->lock);
}

static void tg_deref(tg_t *t) {
    acquire(&t->lock);
    t->ref--;
    release(&t->lock);
}

tg_t *tg_new(proc_t *p) {
    tg_t *newtg = (tg_t *)kalloc();
    if (NULL == newtg) {
        return NULL;
    }

    initlock(&newtg->lock,"threadgroup");
    INIT_LIST_HEAD(&newtg->member);
    newtg->tg_id = p->pid;
    tg_join(newtg,p);
    
    // TODO
    // newtg->futex_wq = (wq_t)INIT_WAIT_QUEUE(newtg->futex_wq);

    return newtg;
}

int tg_thrd_cnt(tg_t *t) {
    int ans = 0;

    acquire(&t->lock);
    ans = t->thrdcnt;
    release(&t->lock);

    return ans;
}

void tg_join(tg_t *t, proc_t *p) {
    acquire(&t->lock);
    t->thrdcnt++;
    // TODO
    // list_add_tail(&p->thrd_head, &t->member);
    release(&t->lock);
}

int tg_quit(tg_t *t) {
    int rest;
    proc_t *p = myproc();
    acquire(&t->lock);
    rest = --t->thrdcnt;
    // TODO
    // list_del_init(&p->thrd_head);
    release(&t->lock);

    return rest;
}

int tg_pid(tg_t *t) {
    return t->tg_id;
}

void tg_free(tg_t **pself) {
    if(pself == NULL || *pself == NULL)
        return;
    tg_t *t = *pself;

    if(t->ref == 0)
        panic("ref");
        
    tg_deref(t);

    if(t->ref > 0)
        return;

    kfree_safe(pself);
}