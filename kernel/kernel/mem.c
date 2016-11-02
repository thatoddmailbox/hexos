#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <multiboot.h>

#include <kernel/idt.h>
#include <kernel/mem.h>

#include <kernel/vga.h>

// TODO: this is soooo architecture-dependent and should be in the arch/i386/ folder but i'll do that *later*
// also TODO: maybe at some point far far away PAE support?

#define CELL_BITS (8*sizeof(*freemap))
#define PAGE_BITS 12

void * mem_start = 0x00000000;

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

uint32_t usable_memory_size = 0;
uint32_t total_memory_size = 0;

static uint32_t pages_free = 0;
static uint32_t pages_total = 0;

static uint32_t *freemap=0;
static uint32_t freemap_bits=0;
static uint32_t freemap_bytes=0;
static uint32_t freemap_cells=0;
static uint32_t freemap_pages=0;

static void * alloc_memory_start = (void*) ALLOC_MEMORY_START;

void page_fault(struct regs r) {
	dbgprint("Unhandled page fault\n");
	panic("Unhandled page fault");
}

bool mem_check_avail(multiboot_info_t * mb_info) {
	printf("checking available memory...\n");

	multiboot_memory_map_t* mmap = mb_info->mmap_addr;
	while(mmap < mb_info->mmap_addr + mb_info->mmap_length) {
		uint32_t end = mmap->base_addr + mmap->length;
		printf("from 0x%x ", mmap->base_addr);
		printf("to 0x%x", end);
		printf(" | %d bytes | %d\n", mmap->length, mmap->type);

		if (mmap->type == 1) {
			usable_memory_size += mmap->length;
		}
		total_memory_size += mmap->length;

		mmap = (multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int) );
	}
	printf("usable memory: %d MiB (%d bytes)\n", usable_memory_size / 1024 / 1024, usable_memory_size);
	printf("total memory: %d MiB (%d bytes)\n", total_memory_size / 1024 / 1024, total_memory_size);
	return true;
}

bool mem_init(multiboot_info_t * mb_info) {
	if (!mem_check_avail(mb_info)) {
		return false;
	}

	pages_total = (usable_memory_size*1024)/(PAGE_SIZE/1024);
	pages_free = pages_total;

	freemap = alloc_memory_start;
	freemap_bits = pages_total;
	freemap_bytes = 1+freemap_bits/8;
	freemap_cells = 1+freemap_bits/CELL_BITS;
	freemap_pages = 1+freemap_bytes/PAGE_SIZE;

	//memset(freemap,0xff,freemap_bytes);
	//for(int i=0;i<freemap_pages;i++) memory_alloc_page(0);

	freemap[0] = 0x0; // something hacky for VMWare

	return true;
}

uint32_t memory_pages_free() {
	return pages_free;
}

uint32_t memory_pages_total() {
	return pages_total;
}

void * memory_alloc_page(bool zeroit) {
	uint32_t i,j;
	uint32_t cellmask;
	uint32_t pagenumber;
	void * pageaddr;

	if(!freemap) {
		printf("memory: not initialized yet!\n");
		return 0;
	}

	for(i=0;i<freemap_cells;i++) {
		if(freemap[i]!=0) {
			for(j=0;j<CELL_BITS;j++) {
				cellmask = (1<<j);
				if(freemap[i]&cellmask) {
					freemap[i] &= ~cellmask;
					pagenumber = i*CELL_BITS+j;
					pageaddr = (pagenumber<<PAGE_BITS)+alloc_memory_start;
					if(zeroit) memset(pageaddr,0,PAGE_SIZE);
					pages_free--;
					return pageaddr;
				}
			}
		}
	}

	panic("memory_alloc_page: Out of memory!");

	return 0;
}

void memory_free_page(void *pageaddr) {
	uint32_t pagenumber = (pageaddr-alloc_memory_start)>>PAGE_BITS;
	uint32_t cellnumber = pagenumber/CELL_BITS;
	uint32_t celloffset = pagenumber%CELL_BITS;
	uint32_t cellmask = (1<<celloffset);
	freemap[cellnumber] |= cellmask;
	pages_free++;
}

void paging_init() {
	printf("[paging] setting up...\n");

	pagedirectory_t * pagedir = (pagedirectory_t *) memory_alloc_page(1); // allocate new page directory
	// fill that directory with entries
	for (uint32_t dir_i = 0; dir_i < 3; dir_i++) {
		pagedirectory_entry_t * dir_entry = &pagedir->pages[dir_i];
		dir_entry->present = 1;
		dir_entry->read_write = 1;
		pagetable_t * pagetable = (pagetable_t *) memory_alloc_page(1); // create a page table
		for (uint32_t table_i = 0; table_i < 1024; table_i++) { // fill that up
			pagetable_entry_t * table_entry = &pagetable->pages[table_i];
			table_entry->present = 1;
			table_entry->read_write = 1;
			table_entry->address = ((0x400000 * dir_i) + (4096 * table_i)) >> 12; // point to the physical memory
		}
		printf("pagetable: 0x%x\n", pagetable);
		dir_entry->address = ((uint32_t) pagetable) >> 12;//pagetable; // point to the new table
	}

	printf("[paging] structs are ready\n");

	paging_load_directory(pagedir);
	paging_enable();

	printf("[paging] done!\n");
}

pagedirectory_t * paging_load_directory(pagedirectory_t *p) {
	asm("xchgw %bx, %bx"); // hi bochs!
	pagedirectory_t *oldp;
	asm("mov %%cr3, %0" : "=r" (oldp));
	asm("mov %0, %%cr3" :: "r" (p));
	return oldp; // return the old directory
}

void paging_refresh() {
	// refreshes the TLB. this should be done when a change to a page is made
	// TODO: change this to use invlpg? Would require detection of target system (invlpg is only on i486 and up)...
	asm("mov %cr3, %eax");
	asm("mov %eax, %cr3");
}

void paging_enable() {
	// set the PG bit in CR0 to enable paging
	printf("pagetable_enable 1\n");
	asm("xchgw %bx, %bx");
	asm("movl %cr0, %eax");
	asm("orl $0x80000000, %eax");
	asm("movl %eax, %cr0");
	printf("pagetable_enable 2\n");
}
