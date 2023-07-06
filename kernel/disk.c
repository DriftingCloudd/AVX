#include "include/types.h"
#include "include/param.h"
#include "include/memlayout.h"
#include "include/riscv.h"
#include "include/printf.h"
#include "include/buf.h"

#ifndef QEMU
#include "include/sdcard.h"
#include "include/dmac.h"
#include "include/sd_final.h"
#else
#include "include/virtio.h"
#endif 

void disk_init(void)
{
    #ifdef QEMU
    virtio_disk_init();
	#else 
	//sdcard_init();
    sd_init();
    #endif
}

void disk_read(struct buf *b)
{
    #ifdef QEMU
	virtio_disk_rw(b, 0);
    #else 
	//sdcard_read_sector(b->data, b->sectorno);
    sd_read((uint32*)b->data, 128, b->sectorno);
	#endif
}

void disk_write(struct buf *b)
{
    #ifdef QEMU
	virtio_disk_rw(b, 1);
    #else 
	//sdcard_write_sector(b->data, b->sectorno);
    sd_write((uint32*)b->data, 128, b->sectorno);
	#endif
}

void disk_intr(void)
{
    #ifdef QEMU
    virtio_disk_intr();
    #else
    printf("should not have disk intr");
    //dmac_intr(DMAC_CHANNEL0);
    #endif
}
