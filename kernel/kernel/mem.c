#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <multiboot.h>

#include <kernel/mem.h>

void * mem_start = 0x00000000;

bool mem_check_avail(multiboot_info_t * mb_info) {
	printf("checking available memory...\n");

	int size = 0;
	multiboot_memory_map_t* mmap = mb_info->mmap_addr;
	while(mmap < mb_info->mmap_addr + mb_info->mmap_length) {
		char inf_buf[33];
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

bool mem_init(multiboot_info_t * mb_info) {
	// make an initial block of 128 bytes
	block_header * initial = (block_header *) mem_start;
	initial->magic = MEM_MAGIC;
	initial->next_page = mem_start + sizeof(block_header) + 128;
	initial->size = 128;
	initial->is_free = false;

	if (!mem_check_avail(mb_info)) {
		return false;
	}

	return true;
}

void * hex_malloc(size_t size) {
	//printf("hexy_hello\n");
	block_header * check = (block_header *) mem_start; // start here
	while (1) {
 		if (check->magic != MEM_MAGIC) {
			// TODO: check if we have space
			printf("no block here, making one\n");
			check->magic = MEM_MAGIC;
			check->next_page = mem_start + sizeof(block_header) + size;
			check->size = size;
			check->is_free = false;
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
