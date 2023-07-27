#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#include "types.h"
#include "printf.h"
#include "spinlock.h"
#include "string.h"


#define RING_BUFFER_SIZE 4095
#pragma pack(8)
struct ring_buffer {
	size_t size;		// for future use
	int head;		// read from head
	int tail;		// write from tail
	// char buf[RING_BUFFER_SIZE + 1]; // left 1 byte
	char buf[RING_BUFFER_SIZE + 1];
};
#pragma pack()

static struct spinlock ring_buffer_lock;

int wait_ring_buffer_read(struct ring_buffer *rbuf, time_t final_ticks);
int wait_ring_buffer_write(struct ring_buffer *rbuf, time_t final_ticks);

static inline void init_ring_buffer(struct ring_buffer *rbuf)
{
	// there is always one byte which should not be read or written
	// memset(rbuf, 0, sizeof(struct ring_buffer)); /* head = tail = 0 */
	memset(rbuf, 0, RING_BUFFER_SIZE); /* head = tail = 0 */
	rbuf->size = RING_BUFFER_SIZE;
	// WH ADD
	// rbuf->buf = kmalloc(PAGE_SIZE);
	initlock(&ring_buffer_lock, "ring_buffer_lock");
	return ;
}

static inline int ring_buffer_used(struct ring_buffer *rbuf)
{
	return (rbuf->tail - rbuf->head + rbuf->size) % (rbuf->size);
}

static inline int ring_buffer_free(struct ring_buffer *rbuf)
{
	// let 1 byte to distinguish empty buffer and full buffer
	return rbuf->size - ring_buffer_used(rbuf) - 1;
}

static inline int ring_buffer_empty(struct ring_buffer *rbuf)
{
	return ring_buffer_used(rbuf) == 0;
}

static inline int ring_buffer_full(struct ring_buffer *rbuf)
{
	return ring_buffer_free(rbuf) == 0;
}

#ifndef min
#define min(x,y) ((x)<(y) ? (x) : (y))
#endif

static inline size_t read_ring_buffer(struct ring_buffer *rbuf, char *buf, size_t size)
{
	acquire(&ring_buffer_lock);
	int len = min(ring_buffer_used(rbuf), size);
	if (len > 0) {
		if (rbuf->head + len > rbuf->size) {
			int right = rbuf->size - rbuf->head,
				left = len - right;
			memcpy(buf, rbuf->buf + rbuf->head, right);
			memcpy(buf + right, rbuf->buf, left);
		}
		else {
			memcpy(buf, rbuf->buf + rbuf->head, len);
		}

		rbuf->head = (rbuf->head + len) % (rbuf->size);
	}
	else if (len < 0)
		panic("read_ring_buffer: len < 0");
	release(&ring_buffer_lock);
	return len;
}

// rbuf should have enough space for buf
static inline size_t write_ring_buffer(struct ring_buffer *rbuf, char *buf, size_t size)
{
	acquire(&ring_buffer_lock);
	int len = min(ring_buffer_free(rbuf), size);
	if (len > 0){
		if (rbuf->tail + len > rbuf->size) {
			int right = rbuf->size - rbuf->tail,
				left = len - right;
			memcpy(rbuf->buf + rbuf->tail, buf, right);
			if (left > 0)
				memcpy(rbuf->buf, buf + right, left);
		}
		else {
			memcpy(rbuf->buf + rbuf->tail, buf, len);
		}

		rbuf->tail = (rbuf->tail + len) % (rbuf->size);
	}
	else if (len < 0)
		panic("read_ring_buffer: len < 0");
	release(&ring_buffer_lock);
	return len;
}



#endif
