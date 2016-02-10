#ifndef _KERNEL_MEM_H
#define _KERNEL_MEM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#define MEM_MAGIC 0xdeadbeef

typedef struct block_header {
	int magic;
	void * next_page;
	int size;
	bool is_free;
} block_header;

bool mem_init();
void * hex_malloc(size_t size);

#endif
