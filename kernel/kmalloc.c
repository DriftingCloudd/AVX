// author: @zbtrs

#include "include/types.h"
#include "include/param.h"
#include "include/memlayout.h"
#include "include/riscv.h"
#include "include/spinlock.h"
#include "include/kalloc.h"
#include "include/string.h"
#include "include/kmalloc.h"
#include "include/printf.h"

void 			kmallocinit(void);
void*           kmalloc(uint size);
void            kfree(void *addr);
