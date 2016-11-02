#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <multiboot.h>

#include <kernel/idt.h>
#include <kernel/mem.h>

#include <kernel/vga.h>

/*
 * Some of this code is from/based off of basekernel.
 * You can see the specific file here: https://github.com/dthain/basekernel/blob/6fff9df12906787b16ba94d685c3ec5bf28eb1eb/src/memory.c
 */

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

struct pageentry {
	unsigned present:1;	// 1 = present
	unsigned readwrite:1;	// 1 = writable
	unsigned user:1;	// 1 = user mode
	unsigned writethrough:1; // 1 = write through

	unsigned nocache:1;	// 1 = no caching
	unsigned accessed:1;	// 1 = accessed
	unsigned dirty:1;	// 1 = dirty
	unsigned pagesize:1;	// leave to zero

	unsigned globalpage:1;	// 1 if not to be flushed
	unsigned avail:3;

	unsigned addr:20;
};

struct pagetable {
	struct pageentry entry[ENTRIES_PER_TABLE];
};

struct pagetable * pagetable_create()
{
	return memory_alloc_page(1);
}

void pagetable_init( struct pagetable *p )
{
	unsigned i,stop;
	stop = usable_memory_size*1024*1024;
	for(i=0;i<stop;i+=PAGE_SIZE) {
		pagetable_map(p,i,i,PAGE_FLAG_KERNEL|PAGE_FLAG_READWRITE);
	}
	stop = (unsigned)VGA_MEMORY+VGA_WIDTH*VGA_HEIGHT*3;
	for(i=(unsigned)VGA_MEMORY;i<=stop;i+=PAGE_SIZE) {
		pagetable_map(p,i,i,PAGE_FLAG_KERNEL|PAGE_FLAG_READWRITE);
	}
}

int pagetable_getmap( struct pagetable *p, unsigned vaddr, unsigned *paddr )
{
	struct pagetable *q;
	struct pageentry *e;

	unsigned a = vaddr>>22;
	unsigned b = (vaddr>>12) & 0x3ff;

	e = &p->entry[a];
	if(!e->present) return 0;

	q = (struct pagetable*) (e->addr << 12);

	e = &q->entry[b];
	if(!e->present) return 0;

	*paddr = e->addr << 12;

	return 1;
}

int pagetable_map( struct pagetable *p, unsigned vaddr, unsigned paddr, int flags )
{
	struct pagetable *q;
	struct pageentry *e;

	unsigned a = vaddr>>22;
	unsigned b = (vaddr>>12) & 0x3ff;

	if(flags&PAGE_FLAG_ALLOC) {
		paddr = (unsigned) memory_alloc_page(0);
		if(!paddr) return 0;
	}

	e = &p->entry[a];

	if(!e->present) {
		q = pagetable_create();
		if(!q) return 0;
		e->present = 1;
		e->readwrite = 1;
		e->user = (flags&PAGE_FLAG_KERNEL) ? 0 : 1;
		e->writethrough = 0;
		e->nocache = 0;
		e->accessed = 0;
		e->dirty = 0;
		e->pagesize = 0;
		e->globalpage = (flags&PAGE_FLAG_KERNEL) ? 1 : 0;
		e->avail = 0;
		e->addr = (((unsigned)q) >> 12);
	} else {
		q = (struct pagetable*) (((unsigned)e->addr) << 12);
	}


	e = &q->entry[b];

	e->present = 1;
	e->readwrite = (flags&PAGE_FLAG_READWRITE) ? 1 : 0;
	e->user = (flags&PAGE_FLAG_KERNEL) ? 0 : 1;
	e->writethrough = 0;
	e->nocache = 0;
	e->accessed = 0;
	e->dirty = 0;
	e->pagesize = 0;
	e->globalpage = !e->user;
	e->avail = (flags&PAGE_FLAG_ALLOC) ? 1 : 0;
	e->addr = (paddr >> 12);

	return 1;
}

void pagetable_unmap( struct pagetable *p, unsigned vaddr )
{
	struct pagetable *q;
	struct pageentry *e;

	unsigned a = vaddr>>22;
	unsigned b = vaddr>>12 & 0x3ff;

	e = &p->entry[a];
	if(e->present) {
		q = (struct pagetable *)(e->addr << 12);
		e = &q->entry[b];
		e->present = 0;
	}
}

void pagetable_delete( struct pagetable *p )
{
	unsigned i,j;

	struct pageentry *e;
	struct pagetable *q;

	for(i=0;i<ENTRIES_PER_TABLE;i++) {
		e = &p->entry[i];
		if(e->present) {
			q = (struct pagetable *) (e->addr<<12);
			for(j=0;j<ENTRIES_PER_TABLE;j++) {
				e = &q->entry[i];
				if(e->present && e->avail) {
					void *paddr;
					paddr = (void *) (e->addr<<12);
					memory_free_page(paddr);
				}
			}
			memory_free_page(q);
		}
	}
}

void pagetable_alloc( struct pagetable *p, unsigned vaddr, unsigned length, int flags )
{
	unsigned npages = length/PAGE_SIZE;

	if(length%PAGE_SIZE) npages++;

	vaddr &= 0xfffff000;

	while(npages>0) {
		unsigned paddr;
		if(!pagetable_getmap(p,vaddr,&paddr)) {
			pagetable_map(p,vaddr,0,flags|PAGE_FLAG_ALLOC);
		}
		vaddr += PAGE_SIZE;
		npages--;
	}
}

struct pagetable * pagetable_load( struct pagetable *p )
{
	asm("xchgw %bx, %bx");
	struct pagetable *oldp;
	asm("mov %%cr3, %0" : "=r" (oldp));
	asm("mov %0, %%cr3" :: "r" (p));
	return oldp;
}

void pagetable_refresh()
{
	asm("mov %cr3, %eax");
	asm("mov %eax, %cr3");
}

void pagetable_enable()
{
	printf("pagetable_enable 1\n");
	asm("xchgw %bx, %bx");
	asm("movl %cr0, %eax");
	asm("orl $0x80000000, %eax");
	asm("movl %eax, %cr0");
	printf("pagetable_enable 2\n");
}

void pagetable_copy( struct pagetable *sp, unsigned saddr, struct pagetable *tp, unsigned taddr, unsigned length );
