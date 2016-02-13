#include <kernel/blockdevice.h>

void atapi_block_read(block_device * this, void * buffer, int number_of_blocks, int offset) {
	atapi_read(((ata_metadata*)(this->metadata))->ata_number, buffer, number_of_blocks, offset);
}
