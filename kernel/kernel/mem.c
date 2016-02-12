#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <multiboot.h>

#include <kernel/idt.h>
#include <kernel/mem.h>

void * mem_start = 0x00000000;

uint32_t page_directory[1024] __attribute__((aligned(4096)));
uint32_t first_page_table[1024] __attribute__((aligned(4096)));

void page_fault(struct regs r) {
	dbgprint("Page fault\n");
	panic("Page fault");
}

bool mem_check_avail(multiboot_info_t * mb_info) {
	printf("checking available memory...\n");

	int size = 0;
	multiboot_memory_map_t* mmap = mb_info->mmap_addr;
	while(mmap < mb_info->mmap_addr + mb_info->mmap_length) {
		char inf_buf[10];
		int inf_int = mmap->size;

		itoa(inf_int, inf_buf, 10);
		printf(inf_buf);

		printf(" | ");

		inf_int = mmap->type;
		itoa(inf_int, inf_buf, 10);
		printf(inf_buf);

		printf("\n");

		if (mmap->type == 1) {
			size += mmap->size;
		}

		mmap = (multiboot_memory_map_t*) ( (unsigned int)mmap + mmap->size + sizeof(unsigned int) );
	}
	return true;
}

void * hex_malloc(size_t size) {
	//printf("hexy_hello\n");
	block_header * check = (block_header *) mem_start; // start here
	while (1) {
		if (check->start_magic != MEM_START_MAGIC) {
			// TODO: check if we have space
			printf("no block here, making one\n");
			check->start_magic = MEM_START_MAGIC;
			check->next_page = mem_start + sizeof(block_header) + size;
			check->size = size;
			check->is_free = false;
			check->end_magic = MEM_END_MAGIC;
			return (void *) check + sizeof(block_header);
		}
		if (check->is_free) {
			// it's free!
			printf("found free block\n");
			return (void *) check + sizeof(block_header);
		}
		printf("trying next block\n");
		check = check + check->size + sizeof(block_header); // go to the next one
	}
}

bool mem_init(multiboot_info_t * mb_info) {
	if (!mem_check_avail(mb_info)) {
		return false;
	}

	unsigned int i;
	for(i = 0; i < 1024; i++) {
		// This sets the following flags to the pages:
		//   Supervisor: Only kernel-mode can access them
		//   Write Enabled: It can be both read from and written to
		//   Not Present: The page table is not present
		page_directory[i] = 0x00000002;
	}

	//we will fill all 1024 entries in the table, mapping 4 megabytes
	for(i = 0; i < 1024; i++)
	{
		// As the address is page aligned, it will always leave 12 bits zeroed.
		// Those bits are used by the attributes ;)
		first_page_table[i] = (i * 0x1000) | 3; // attributes: supervisor level, read/write, present.
	}

	// and put the table in the directory
	page_directory[0] = ((unsigned int)first_page_table) | 3;

	load_page_directory(page_directory);
	enable_paging();

	return true;
}
