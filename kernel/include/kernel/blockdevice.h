#ifndef _KERNEL_BLOCKDEVICE_H
#define _KERNEL_BLOCKDEVICE_H

#include <stdint.h>

typedef void (*read_block_t) (void * this, void * buffer, int number_of_blocks, int offset);

typedef struct {
	read_block_t * read_block;
	void * metadata;
} block_device;

typedef struct {
	uint8_t ata_number
} ata_metadata;

void atapi_block_read(block_device * this, void * buffer, int number_of_blocks, int offset);

#endif
