#ifndef _KERNEL_HEAP_H
#define _KERNEL_HEAP_H

typedef struct _kernel_heap_block_bm {
	struct _kernel_heap_block_bm *next;
	uint32_t size;
	uint32_t used;
	uint32_t bsize;
	uint32_t lfb;
} kernel_heap_block_bm;

typedef struct _kernel_heap {
	kernel_heap_block_bm *fblock;
} kernel_heap;

void kheap_init(kernel_heap *heap);
int kheap_add_block(kernel_heap *heap, uint32_t addr, uint32_t size, uint32_t bsize);
void * kalloc(kernel_heap *heap, uint32_t size);
void kfree(kernel_heap *heap, void *ptr);

extern kernel_heap main_heap;

#endif
