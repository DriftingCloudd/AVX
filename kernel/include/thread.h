#ifndef _Thread_H_
#define _Thread_H_

#include "types.h"
#include "proc.h"
#include "fat32.h"
#include "types.h"
#include "queue.h"

typedef struct Trapframe {
    uint64 kernelSatp;
    uint64 kernelSp;
    uint64 trapHandler;
    uint64 epc;
    uint64 kernelHartId;
    uint64 ra;
    uint64 sp;
    uint64 gp;
    uint64 tp;
    uint64 t0;
    uint64 t1;
    uint64 t2;
    uint64 s0;
    uint64 s1;
    uint64 a0;
    uint64 a1;
    uint64 a2;
    uint64 a3;
    uint64 a4;
    uint64 a5;
    uint64 a6;
    uint64 a7;
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
    uint64 t3;
    uint64 t4;
    uint64 t5;
    uint64 t6;
    uint64 ft0;
    uint64 ft1;
    uint64 ft2;
    uint64 ft3;
    uint64 ft4;
    uint64 ft5;
    uint64 ft6;
    uint64 ft7;
    uint64 fs0;
    uint64 fs1;
    uint64 fa0;
    uint64 fa1;
    uint64 fa2;
    uint64 fa3;
    uint64 fa4;
    uint64 fa5;
    uint64 fa6;
    uint64 fa7;
    uint64 fs2;
    uint64 fs3;
    uint64 fs4;
    uint64 fs5;
    uint64 fs6;
    uint64 fs7;
    uint64 fs8;
    uint64 fs9;
    uint64 fs10;
    uint64 fs11;
    uint64 ft8;
    uint64 ft9;
    uint64 ft10;
    uint64 ft11;
} Trapframe;
/*
typedef struct SignalContext {
    Trapframe contextRecover;
	bool start;
	uint8 signal;
    ucontext* uContext;
	LIST_ENTRY(SignalContext) link;
} SignalContext;
*/
typedef struct Thread {
    Trapframe trapframe;
    LIST_ENTRY(Thread) link;
    uint64 awakeTime;
    uint32 id;
    LIST_ENTRY(Thread) scheduleLink;
    enum procstate state;
    struct spinlock lock;
    uint64 chan;//wait Object
    uint64 currentKernelSp;
    int reason;
    uint32 retValue;
    uint64 setChildTid;
    uint64 clearChildTid;
    struct proc* process;
    uint64 robustHeadPointer;
    int killed;
	//struct SignalContextList waitingSignal;
} Thread;

LIST_HEAD(ThreadList, Thread);

Thread* myThread(); // Get current running thread in this hart
void threadFree(Thread *th);
uint64 getThreadTopSp(Thread* th);

void threadInit();
int mainThreadAlloc(Thread **new_thread, uint64 parentId);
int threadAlloc(Thread **new_thread, struct proc* process, uint64 userSp);
int tid2Thread(uint32 threadId, struct Thread **thread, int checkPerm);
void threadRun(Thread* thread);
void threadDestroy(Thread* thread);
void wakeup_thread(void *chan);

#define NORMAL         0
#define KERNEL_GIVE_UP 1
#define SELECT_BLOCK   2

#define LOCALE_NAME_MAX 23

struct __locale_map {
	const void *map;
	uint64 map_size;
	char name[LOCALE_NAME_MAX+1];
	const struct __locale_map *next;
};

struct __locale_struct {
	const struct __locale_map *cat[6];
};

struct pthread {
	/* Part 1 -- these fields may be external or
	 * internal (accessed via asm) ABI. Do not change. */
	struct pthread *self;
	uint64 *dtv;
	struct pthread *prev, *next; /* non-ABI */
	uint64 sysinfo;
	uint64 canary, canary2;

	/* Part 2 -- implementation details, non-ABI. */
	int tid;
	int errno_val;
	volatile int detach_state;
	volatile int cancel;
	volatile unsigned char canceldisable, cancelasync;
	unsigned char tsd_used:1;
	unsigned char dlerror_flag:1;
	unsigned char *map_base;
	uint64 map_size;
	void *stack;
	uint64 stack_size;
	uint64 guard_size;
	void *result;
	struct __ptcb *cancelbuf;
	void **tsd;
	struct {
		volatile void *volatile head;
		long off;
		volatile void *volatile pending;
	} robust_list;
	volatile int timer_id;
	struct __locale_struct *locale;
	volatile int killlock[1];
	char *dlerror_buf;
	void *stdio_locks;

	/* Part 3 -- the positions of these fields relative to
	 * the end of the structure is external and internal ABI. */
	uint64 canary_at_end;
	uint64 *dtv_copy;
};

#endif