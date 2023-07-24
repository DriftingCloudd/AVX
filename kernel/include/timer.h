#ifndef __TIMER_H
#define __TIMER_H

#include "types.h"
#include "spinlock.h"

extern struct spinlock tickslock;
extern uint ticks;

typedef struct TimeSpec {
    uint64 second;
    uint64 microSecond;    
} TimeSpec;

struct tms              
{                     
	long tms_utime;  
	long tms_stime;  
	long tms_cutime; 
	long tms_cstime; 
};

struct timespec2 {
	long tv_sec;	// seconds
	long tv_nsec;	// nanoseconds
};

typedef struct timespec2 TimeSpec2;

void timerinit();
void set_next_timeout();
void timer_tick();

#endif
