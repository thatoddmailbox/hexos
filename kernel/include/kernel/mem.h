#ifndef _KERNEL_MEM_H
#define _KERNEL_MEM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/idt.h>

#define MEM_MAGIC 0xdeadbeef

typedef struct block_header {
	int magic;
	void * next_page;
	int size;
	bool is_free;
} block_header;

typedef struct multiboot_memory_map {
	unsigned int size;
	unsigned int base_addr_low,base_addr_high;
	unsigned int length_low,length_high;
	unsigned int type;
} multiboot_memory_map_t;

// these are defined in boot.s
extern void load_page_directory(unsigned int*);
extern void enable_paging();

bool mem_init();

#endif
