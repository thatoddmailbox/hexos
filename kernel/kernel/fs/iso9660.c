#include <kernel/blockdevice.h>
#include <kernel/fs/iso9660.h>
#include <kernel/heap.h>
#include <kernel/vfs.h>

#include <stdbool.h>

void iso9660_init_volume(block_device * dev, fs_node_t * target, fs_node_t * target_parent) {
	// TODO: free memory and stuff
	void * buffer = kalloc(&main_heap, 2048);
	((read_block_t)dev->read_block)(dev, buffer, 1, ISO9660_OFFSET);
	iso9660_primary_volume_descriptor_t * vol_desc = (iso9660_primary_volume_descriptor_t *) buffer;
	printf("iso9660: volume identifier: %s\n", vol_desc->volume_identifier);

	kfree(&main_heap, buffer);

	void * buffer2 = kalloc(&main_heap, 2048);
	((read_block_t)dev->read_block)(dev, buffer2, 1, vol_desc->location_of_typel_path_table);

	iso9660_path_table_entry_t * entry = (iso9660_path_table_entry_t *) buffer2;

	target->inode = entry->lba;
	target->impl = (uint32_t) dev; // load the root directory's lba into the mountpoint

	strcpy(target->name, "mnt");
	target->flags = FS_DIRECTORY;
	target->parent = target_parent;
	target->readdir = &iso9660_readdir;
	target->finddir = &iso9660_finddir;

	// read in the root directory's info
	((read_block_t)dev->read_block)(dev, buffer, 1, entry->lba);

	iso9660_directory_entry_t * dir_entry = (iso9660_directory_entry_t *) buffer;
	bool skip_first = true;
	while (1) {
		if (dir_entry->length_of_record == 0) {
			printf("iso9660: stopping, len == 0\n");
			break;
		}
		if (!skip_first && dir_entry->file_identifier_and_system_use[0] != '\1') { // why are these things
			printf("iso9660: file in root dir: ");
			terminal_write(dir_entry->file_identifier_and_system_use, dir_entry->length_of_file_identifier);
			printf(" - len is %d bytes\n", dir_entry->length_of_file);
		}
		if (skip_first) {
			skip_first = false;
		}
		buffer += dir_entry->length_of_record;
		dir_entry = (iso9660_directory_entry_t *) buffer;
	}

	/*uint16_t i = 1;
	while (1) {
		if (entry->length_of_directory_identifier == 0) {
			printf("iso9660: stopping, dir iden len == 0\n");
			break;
		}
		printf("iso9660: directory_identifier #%d: ", i);
		terminal_write(entry->directory_identifier, entry->length_of_directory_identifier);
		printf(" (parentnum = %d, len = %d, lba = %d)\n", entry->parent_dir_num, entry->length_of_directory_identifier, entry->lba);

		// somehow it doesn't like adding to entry, so add to buffer2
		// and set entry = buffer2
		buffer2 += 8;
		buffer2 += entry->length_of_directory_identifier;
		if (entry->length_of_directory_identifier % 2 == 1) {
			buffer2 += 1; // padding byte
		}
		entry = (iso9660_path_table_entry_t *) buffer2;

		i++;
	}*/

	//iso9660_path_table_entry_t * entry2 = buffer2 + ISO9660_PATH_TABLE_OFFSET;
	//printf("iso9660: iden: %s\n", entry->directory_identifier);
}

dirent iso9660_resp;
dirent * iso9660_readdir(fs_node_t * this, uint32_t i) {
	// TODO: make this more efficient
	// TODO: this currently will take exponentially longer as the files in a directory goes up
	// TODO: shouldn't be too hard to fix...

	void * buffer = kalloc(&main_heap, 2048);
	block_device * dev = (block_device *) this->impl;
	((read_block_t)dev->read_block)(dev, buffer, 1, this->inode);

	iso9660_directory_entry_t * dir_entry = (iso9660_directory_entry_t *) buffer;
	bool skip_first = true;
	uint32_t count = 0;

	while (1) {
		if (dir_entry->length_of_record == 0) {
			kfree(&main_heap, buffer);
			return 0;
		}
		if (!skip_first && dir_entry->file_identifier_and_system_use[0] != '\1') { // why are these things
			if (count == i) { // is it the right number?
				// yay, let's fill in a dirent and send it back
				if (dir_entry->length_of_file_identifier <= 255) {
					dir_entry->file_identifier_and_system_use[dir_entry->length_of_file_identifier] = '\0';
					strcpy(iso9660_resp.d_name, dir_entry->file_identifier_and_system_use);
				} else {
					// let's not buffer overflow
					strcpy(iso9660_resp.d_name, "error");
					iso9660_resp.d_name[6] = 0;
				}
				iso9660_resp.d_ino = 0;
				kfree(&main_heap, buffer);
				return &iso9660_resp;
			}
			count += 1;
		}
		if (skip_first) {
			skip_first = false;
		}
		buffer += dir_entry->length_of_record;
		dir_entry = (iso9660_directory_entry_t *) buffer;
	}

	// lolwut
	// how did you get here? let's just return 0 before stuff gets weirder
	dbgprint("iso9660: this shouldn't happen\n");
	kfree(&main_heap, buffer);
	return 0;
}

fs_node_t * iso9660_finddir(fs_node_t * this, char * name) {
	return 0;
}
