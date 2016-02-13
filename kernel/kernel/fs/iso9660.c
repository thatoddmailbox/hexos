#include <kernel/blockdevice.h>
#include <kernel/fs/iso9660.h>

void iso9660_init_volume(block_device * dev) {
	// TODO: free memory and stuff
	void * buffer = memory_alloc_page(2048*1);
	((read_block_t)dev->read_block)(dev, buffer, 1, ISO9600_MAIN_OFFSET);
	iso9660_primary_volume_descriptor_t * vol_desc = (iso9660_primary_volume_descriptor_t *) buffer;
	printf("iso9660: volume identifier: %s\n", vol_desc->volume_identifier);

	printf("iso9660: path table loc: %d\n", vol_desc->location_of_typel_path_table);

	void * buffer2 = memory_alloc_page(2048);
	((read_block_t)dev->read_block)(dev, buffer2, 1, vol_desc->location_of_typel_path_table);
	printf("iso9660: path table: ");
	terminal_write(buffer2, 128);
	printf("\n");

	iso9660_path_table_entry_t * entry = (iso9660_path_table_entry_t *) buffer2;

	while (1) {
		if (entry->length_of_directory_identifier == 0) {
			printf("iso9660: stopping, dir iden len == 0\n");
			break;
		}
		printf("iso9660: directory_identifier: ");
		terminal_write(entry->directory_identifier, entry->length_of_directory_identifier);
		printf(" (len = %d)\n", entry->length_of_directory_identifier);

		// somehow it doesn't like adding to entry, so add to buffer2
		// and set entry = buffer2
		buffer2 += 8;
		buffer2 += entry->length_of_directory_identifier;
		if (entry->length_of_directory_identifier % 2 == 1) {
			buffer2 += 1; // padding byte
		}
		entry = (iso9660_path_table_entry_t *) buffer2;
	}

	//iso9660_path_table_entry_t * entry2 = buffer2 + ISO9600_PATH_TABLE_OFFSET;
	//printf("iso9660: iden: %s\n", entry->directory_identifier);
}
