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

typedef struct multiboot_memory_map {
	uint32_t size;
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
} __attribute__((packed)) multiboot_memory_map_t;

void * malloc(size_t size);

bool mem_init();

void * memory_alloc_page( bool zeroit );
void memory_free_page( void *addr );

// paging structs
// an entry in the page TABLE
typedef struct pagetable_entry {
	unsigned present : 1;
	unsigned read_write : 1;
	unsigned usermode_access : 1;
	unsigned writethrough : 1;
	unsigned cache_disabled : 1;
	unsigned accessed : 1;
	unsigned dirty : 1;
	unsigned reserved : 1;
	unsigned global : 1;
	unsigned extra : 3;
	unsigned address : 20;
} __attribute__((packed)) pagetable_entry_t;

typedef struct pagetable {
	pagetable_entry_t pages[1024];
} __attribute__((packed)) pagetable_t;

// an entry in the page DIRECTORY
typedef struct pagedirectory_entry {
	unsigned present : 1;
	unsigned read_write : 1;
	unsigned usermode_access : 1;
	unsigned writethrough : 1;
	unsigned cache_disabled : 1;
	unsigned accessed : 1;
	unsigned reserved : 1;
	unsigned page_size : 1;
	unsigned reserved2 : 1;
	unsigned extra : 3;
	unsigned address : 20;
} __attribute__((packed)) pagedirectory_entry_t;

typedef struct pagedirectory {
	pagedirectory_entry_t pages[1024];
} __attribute__((packed)) pagedirectory_t;

/*struct pagetable * pagetable_create();
void pagetable_init( struct pagetable *p );
int  pagetable_map( struct pagetable *p, unsigned vaddr, unsigned paddr, int flags );
int  pagetable_getmap( struct pagetable *p, unsigned vaddr, unsigned *paddr );
void pagetable_unmap( struct pagetable *p, unsigned vaddr );
void pagetable_alloc( struct pagetable *p, unsigned vaddr, unsigned length, int flags );
void pagetable_delete( struct pagetable *p );*/
void paging_init();

pagedirectory_t * paging_load_directory(pagedirectory_t *p);
void paging_enable();
void paging_refresh();

#endif
