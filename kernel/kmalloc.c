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

#define KMEM_OBJ_MIN_SIZE   ((uint64)32)
#define KMEM_OBJ_MAX_SIZE 	((uint64)4048)
#define KMEM_OBJ_MAX_COUNT  (PGSIZE / KMEM_OBJ_MIN_SIZE)
#define TABLE_END 	255
#define KMEM_TABLE_SIZE 	17
#define _hash(n) \
	((n) % KMEM_TABLE_SIZE)
#define ROUNDUP16(n) \
	(((n) + 15) & ~0x0f)

struct kmem_node {
    struct kmem_node *next;   // 指向下一个kmem节点
    struct {
        uint64 obj_size;   // 节点内每个对象的大小
        uint64 obj_addr;    // 节点内第一个对象的起始地址
    } config;
    uint8 avail;    // 当前这个节点可以分配的对象的数目
    uint8 cnt;     // 当前分配了多少个对象
    uint8 table[KMEM_OBJ_MAX_COUNT];    // 用一个数组来存对象
};

// 一个kmem节点的固定大小
#define KMEM_NODE_FIX \
	(sizeof(struct kmem_node*) + 2 * sizeof(uint64) + 2 * sizeof(int8))

struct kmem_allocator {
    struct spinlock lock;
    uint obj_size;
    uint16 npages;
    uint16 nobjs;
    struct kmem_node *list;
    struct kmem_allocator *next;  // allocator分给不同的线程使用，降低锁的粒度
};

struct kmem_allocator kmem_adam;
struct kmem_allocator *kmem_table[KMEM_TABLE_SIZE];
struct spinlock kmem_table_lock;

// 分配page不是在这里完成的,应该在kinit之后执行
void kmallocinit(void) {
    // 初始化第一个allocator
    initlock(&(kmem_adam.lock), "kmem_adam");
    kmem_adam.list = NULL;
    kmem_adam.next = NULL;
    kmem_adam.npages = kmem_adam.nobjs = 0;
    kmem_adam.obj_size = ROUNDUP16(sizeof(struct kmem_allocator));

    for (uint8 i = 0; i < KMEM_TABLE_SIZE; i++)
        kmem_table[i] = NULL;
    
    int hash = _hash(kmem_adam.obj_size);
    kmem_table[hash] = &kmem_adam;
    initlock(&kmem_table_lock,"kmem_table");
    printf("kmalloc init\n");
}

void *kmalloc(uint size);

static struct kmem_allocator *get_allocator(uint64 raw_size) {
    uint64 roundup_size = ROUNDUP16(raw_size);
    uint64 hash = _hash(roundup_size);

    // 先找一个已经存在的allocator
    for (struct kmem_allocator *tmp = kmem_table[hash]; NULL != tmp; tmp = tmp -> next) {
        if (tmp ->obj_size == roundup_size)
            return tmp;
    }

    acquire(&kmem_table_lock);
    // 如果在获得锁之前其他进程已经创建了这个allocator
    if (NULL != kmem_table[hash] && kmem_table[hash] ->obj_size == roundup_size) {
        release(&kmem_table_lock);
        return kmem_table[hash];
    }

    struct kmem_allocator *tmp = (struct kmem_allocator*)kmalloc(sizeof(struct kmem_allocator));
    if (NULL != tmp) {
        initlock(&(tmp ->lock),"kmem_alloc");
        tmp ->list = NULL;
        tmp ->obj_size = roundup_size;
        tmp ->npages = 0;
        tmp ->nobjs = 0;
        tmp ->next = kmem_table[hash];
        kmem_table[hash] = tmp;
    }
    release(&kmem_table_lock);

    return tmp;
}

void *kmalloc(uint size) {
    if (size < KMEM_OBJ_MIN_SIZE) 
        size = KMEM_OBJ_MIN_SIZE;
    else if (size > KMEM_OBJ_MAX_SIZE)
        return NULL;

    struct kmem_allocator *alloc = get_allocator(size);
    if (NULL == alloc) {
        return NULL;
    }

    acquire(&(alloc->lock));
    if (NULL == alloc ->list) {
        struct kmem_node *tmp = (struct kmem_node*)kalloc();
        if (NULL == tmp) {
            release(&(alloc->lock));
            return NULL;
        }
        alloc ->npages++;

        uint roundup_size = ROUNDUP16(size);
        uint8 capa = (PGSIZE - ROUNDUP16(KMEM_NODE_FIX)) / ((roundup_size) + 1);
        tmp ->next = NULL;
        tmp ->config.obj_size = roundup_size;
        tmp ->config.obj_addr = (uint64)tmp + ROUNDUP16(KMEM_NODE_FIX + capa);
        tmp ->avail = tmp ->cnt = 0;
        for (uint8 i = 0; i < capa - 1; i++) 
            tmp ->table[i] = i + 1;
        tmp ->table[capa - 1] = TABLE_END;
        alloc ->list = tmp;
    }

    alloc ->nobjs++;
    struct kmem_node *node = alloc ->list;
    void *ret = (void *)(node ->config.obj_addr + ((uint64)node->avail) * node ->config.obj_size);
    node ->cnt++;
    node ->avail = node ->table[node ->avail];

    if (TABLE_END == node ->avail)
        alloc ->list = node ->next;
    
    release(&(alloc ->lock));

    return ret;
}

void kmfree(void *addr) {
    struct kmem_node *node = (struct kmem_node*)PGROUNDDOWN((uint64)addr);
	uint8 avail = ((uint64)addr - node->config.obj_addr) / node->config.obj_size;
	struct kmem_allocator *alloc = get_allocator(node->config.obj_size);

	acquire(&(alloc->lock));
	alloc->nobjs--;
	if (TABLE_END == node->avail) {
		node->next = alloc->list;
		alloc->list = node;
	}
	node->table[avail] = node->avail;
	node->avail = avail;
	node->cnt -= 1;
	if (0 == node->cnt) {
		struct kmem_node **pprev = &(alloc->list);
		struct kmem_node *tmp = alloc->list;
		while (NULL != tmp && node != tmp) {
			pprev = &(tmp->next);
			tmp = tmp->next;
		}
		if (NULL == tmp) {
			panic("kmfree()\n");
		}
		*pprev = tmp->next;
		kfree(node);
		alloc->npages--;
	}

	release(&(alloc->lock));
}