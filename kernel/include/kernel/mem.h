#ifndef _KERNEL_MEM_H
#define _KERNEL_MEM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/idt.h>

#define MEM_START_MAGIC 0xdeadbeef
#define MEM_END_MAGIC 0xfeebdaed

#define PAGE_SIZE 4096
#define ENTRIES_PER_TABLE (PAGE_SIZE/4)

#define PAGE_FLAG_USER        0
#define PAGE_FLAG_KERNEL      1
#define PAGE_FLAG_EXISTS      0
#define PAGE_FLAG_ALLOC       2
#define PAGE_FLAG_READONLY    0
#define PAGE_FLAG_READWRITE   4
#define PAGE_FLAG_NOCLEAR     0
#define PAGE_FLAG_CLEAR       8

#define ALLOC_MEMORY_START  0x100000

typedef struct block_header {
	int start_magic; // to verify page
	int size;
	bool is_free;
	void * next_page; // linked list
	int end_magic; // so we can merge nearby blocks
} __attribute__((packed)) block_header;

typedef struct multiboot_memory_map {
	uint32_t size;
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
} __attribute__((packed)) multiboot_memory_map_t;

// these are defined in boot.s
extern void load_page_directory(unsigned int*);
extern void enable_paging();

void * malloc(size_t size);

bool mem_init();

#endif
