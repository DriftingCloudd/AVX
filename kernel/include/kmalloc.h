#ifndef __KALLOC_H
#define __KALLOC_H

#include "types.h"

void 			kmallocinit(void);
void*           kmalloc(uint size);
void            kfree(void *addr);

#endif
