#ifndef _KERNEL_BLOCKDEVICE_H
#define _KERNEL_BLOCKDEVICE_H

typedef void (*read_block_t) (void * buffer, int number_of_blocks, int offset);

typedef struct {
	read_block_t * read_block;
	void * metadata = 0;
} block_device;

typedef struct {
	uint8_t ata_number
} ata_metadata;

#endif
