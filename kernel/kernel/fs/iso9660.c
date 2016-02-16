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
}

dirent iso9660_resp;
fs_node_t iso9660_fsnode_resp;

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

fs_node_t * iso9660_entry_to_node(iso9660_directory_entry_t * dir_entry, block_device * dev, fs_node_t * parent) {
	// TODO: SUSP and Rock Ridge stuff will go here
	// TODO: Joliet? it seems complicated...
	fs_node_t * response = kalloc(&main_heap, sizeof(fs_node_t));

	if (dir_entry->length_of_file_identifier <= 255) {
		strcpy(response->name, dir_entry->file_identifier_and_system_use);
		response->name[dir_entry->length_of_file_identifier] = '\0';
	} else {
		// no buffer overflow plox
		strcpy(response->name, "error");
		response->name[6] = '\0';
	}

	// TODO: all those other flags
	if (dir_entry->flags & ISO9660_FLAG_DIRECTORY) {
		response->flags = FS_DIRECTORY;
	} else {
		response->flags = FS_FILE;
	}

	// TODO: fill this in with info from Rock Ridge, when that's done
	response->mask = 0;
	response->uid = 0;
	response->gid = 0;

	response->length = dir_entry->length_of_file;

	response->inode = dir_entry->lba;
	response->impl = (uint32_t) dev;

	response->parent = parent;

	// TODO: these
	response->read = 0;
	response->write = 0;
	response->open = 0;
	response->close = 0;

	response->readdir = &iso9660_readdir;
	response->finddir = &iso9660_finddir;

	return response;
}

fs_node_t * iso9660_finddir(fs_node_t * current_dir, char * name) {
	//printf("iso9660: finddir: lba of current: %d\n", current_dir->inode);
	void * buffer = kalloc(&main_heap, 2048);
	block_device * dev = (block_device *) current_dir->impl;
	((read_block_t)dev->read_block)(dev, buffer, 1, current_dir->inode);

	iso9660_directory_entry_t * dir_entry = (iso9660_directory_entry_t *) buffer;
	bool skip_first = true;
	uint32_t count = 0;

	while (1) {
		if (dir_entry->length_of_record == 0) {
			kfree(&main_heap, buffer);
			return 0;
		}
		if (!skip_first && dir_entry->file_identifier_and_system_use[0] != '\1') { // why are these things
			if (dir_entry->length_of_file_identifier <= 255) {
				dir_entry->file_identifier_and_system_use[dir_entry->length_of_file_identifier] = '\0';
				if (!strcmp(dir_entry->file_identifier_and_system_use, name)) { // is it the right file?
					// yay, let's fill in a response and send it back
					fs_node_t * response = iso9660_entry_to_node(dir_entry, dev, current_dir);
					kfree(&main_heap, buffer);
					return response;
				}
			} else {
				// blegh no buffer overflow
				dbgprint("iso9660: warning: file identifier too large!\n");
				kfree(&main_heap, buffer);
				return 0;
			}
		}
		if (skip_first) {
			skip_first = false;
		}
		buffer += dir_entry->length_of_record;
		dir_entry = (iso9660_directory_entry_t *) buffer;
	}

	kfree(&main_heap, buffer);
	return 0;
}
