// Copyright (c) 2006-2019 Frans Kaashoek, Robert Morris, Russ Cox,
//                         Massachusetts Institute of Technology

#include "include/types.h"
#include "include/param.h"
#include "include/memlayout.h"
#include "include/riscv.h"
#include "include/sbi.h"
#include "include/console.h"
#include "include/printf.h"
#include "include/kalloc.h"
#include "include/timer.h"
#include "include/trap.h"
#include "include/proc.h"
#include "include/plic.h"
#include "include/vm.h"
#include "include/disk.h"
#include "include/buf.h"
#include "include/sysinfo.h"
#include "include/thread.h"
#ifndef QEMU
#include "include/sd_final.h"
extern void _start(void);
#endif
extern void _entry(void);

static inline void inithartid(unsigned long hartid) {
  asm volatile("mv tp, %0" : : "r" (hartid & 0x1));
}

volatile static int started = 0;
static int first = 0;
extern void boot_stack(void);
extern void boot_stack_top(void);
extern void initlogbuffer(void);

void
main(unsigned long hartid, unsigned long dtb_pa)
{
  inithartid(hartid);
  #ifdef QEMU
  if (first == 0) {
  #else
  if(hartid == 1) {
  #endif
    first = 1;
    cpuinit();
    consoleinit();
    printfinit();   // init a lock for printf 
    #ifdef DEBUG
    print_logo();
    debug_print("hart %d enter main()...\n", hartid);
    #endif
    kinit();         // physical page allocator
    kvminit();       // create kernel page table
    kvminithart();   // turn on paging
    timerinit();     // init a lock for timer
    trapinithart();  // install kernel trap vector, including interrupt handler
    threadInit();  
    procinit();
    plicinit();
    plicinithart();
    disk_init();
    binit();         // buffer cache
    initlogbuffer();
    fileinit();      // file table
    userinit();      // first user process
    debug_print("hart %d init done\n", hartid);
    
    for(int i = 0; i < NCPU; i++) {
      if(i == hartid)
        continue;
      //sbi_send_ipi(mask, 1);
#ifdef QEMU
      sbi_hart_start(i, (unsigned long)_entry, 0);
#else
      //sbi_hart_start(i, (unsigned long)_start, 0);
#endif
    }
    __sync_synchronize();
    started = 1;
  }
  else
  {
    // other hart 
    while (started == 0)
      ;
    __sync_synchronize();
    #ifdef DEBUG
    debug_print("hart %d enter main()...\n", hartid);
    #endif
    kvminithart();
    trapinithart();
    plicinithart();  // ask PLIC for device interrupts
    debug_print("hart 1 init done\n");
  }
  scheduler();
}
