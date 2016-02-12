#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <multiboot.h>

#include <kernel/idt.h>
#include <kernel/mem.h>

void * mem_start = 0x00000000;

uint32_t current_placement_address = 0x0;
uint32_t current_page_directory = 0x0;

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

bool mem_init(multiboot_info_t * mb_info) {
	if (!mem_check_avail(mb_info)) {
		return false;
	}

	return true;
}
