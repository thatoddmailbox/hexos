#ifndef _KERNEL_MEM_H
#define _KERNEL_MEM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/idt.h>

#define MEM_START_MAGIC 0xdeadbeef
#define MEM_END_MAGIC 0xfeebdaed

typedef struct block_header {
	int start_magic; // to verify page
	int size;
	bool is_free;
	void * next_page; // linked list
	int end_magic; // so we can merge nearby blocks
} __attribute__((packed)) block_header;

typedef struct multiboot_memory_map {
	unsigned int size;
	unsigned int base_addr_low,base_addr_high;
	unsigned int length_low,length_high;
	unsigned int type;
} multiboot_memory_map_t;

// these are defined in boot.s
extern void load_page_directory(unsigned int*);
extern void enable_paging();

void * malloc(size_t size);

bool mem_init();

#endif
