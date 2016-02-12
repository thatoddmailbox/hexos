#ifndef _KERNEL_MEM_H
#define _KERNEL_MEM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/idt.h>

#define MEM_MAGIC 0xdeadbeef

#define INDEX_FROM_BIT(a) (a/(8*4))
#define OFFSET_FROM_BIT(a) (a%(8*4))

uint32_t *frames;
uint32_t nframes;

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

typedef struct page {
	uint32_t present    : 1;   // Page present in memory
	uint32_t rw         : 1;   // Read-only if clear, readwrite if set
	uint32_t user       : 1;   // Supervisor level only if clear
	uint32_t accessed   : 1;   // Has the page been accessed since last refresh?
	uint32_t dirty      : 1;   // Has the page been written to since last refresh?
	uint32_t unused     : 7;   // Amalgamation of unused and reserved bits
	uint32_t frame      : 20;  // Frame address (shifted right 12 bits)
} page_t;

typedef struct page_table {
	page_t pages[1024];
} page_table_t;

typedef struct page_directory
{
	page_table_t *tables[1024]; // page_table pointers
	uint32_t tablesPhysical[1024]; // physical pointers to pagetables
	uint32_t physicalAddr; // physical address of tablesPhysical
} page_directory_t;

bool mem_init();

uint32_t kmalloc_int(uint32_t sz, int align, uint32_t *phys);

uint32_t kmalloc_a(uint32_t sz);  // page aligned.
uint32_t kmalloc_p(uint32_t sz, uint32_t *phys); // returns a physical address.
uint32_t kmalloc_ap(uint32_t sz, uint32_t *phys); // page aligned and returns a physical address.
uint32_t kmalloc(uint32_t sz); // vanilla (normal).

void switch_page_directory(page_directory_t *new);
page_t *get_page(uint32_t address, int make, page_directory_t *dir);

void page_fault(struct regs r); // handles page faults

#endif
