#include <kernel/blockdevice.h>
#include <kernel/fs/iso9660.h>
#include <kernel/heap.h>
#include <kernel/vfs.h>

#include <stdbool.h>

void iso9660_init_volume(block_device * dev, fs_node_t * target, fs_node_mini_t target_parent) {
	iso9660_fs * fs_info = kalloc(&main_heap, sizeof(iso9660_fs));

	void * buffer = kalloc(&main_heap, 2048);
	((read_block_t)dev->read_block)(dev, buffer, 1, ISO9660_OFFSET);
	iso9660_primary_volume_descriptor_t * vol_desc = (iso9660_primary_volume_descriptor_t *) buffer;
	//printf("iso9660: volume identifier: %s\n", vol_desc->volume_identifier);

	kfree(&main_heap, buffer);

	void * buffer2 = kalloc(&main_heap, 2048);
	((read_block_t)dev->read_block)(dev, buffer2, 1, vol_desc->location_of_typel_path_table);

	iso9660_path_table_entry_t * entry = (iso9660_path_table_entry_t *) buffer2;

	fs_info->dev = dev;
	fs_info->mountpoint = target_parent;

	target->inode = entry->lba;
	target->impl = (uint32_t) fs_info; // load the root directory's lba into the mountpoint

	strcpy(target->name, "mnt");
	target->flags = FS_DIRECTORY;
	target->parent = target_parent;
	target->readdir = &iso9660_readdir;
	target->finddir = &iso9660_finddir;
}

dirent iso9660_resp;

dirent * iso9660_readdir(fs_node_t * this, uint32_t i) {
	// TODO: make this more efficient
	// TODO: this currently will take exponentially longer as the files in a directory goes up
	// TODO: shouldn't be too hard to fix...

	void * buffer = kalloc(&main_heap, 2048);
	void * buffer_start = buffer;

	iso9660_fs * fs_info = (iso9660_fs *) this->impl;
	((read_block_t)fs_info->dev->read_block)(fs_info->dev, buffer, 1, this->inode);

	iso9660_directory_entry_t * dir_entry = (iso9660_directory_entry_t *) buffer;
	bool skip_first = true;
	uint32_t count = 0;

	while (1) {
		if (dir_entry->length_of_record == 0) {
			kfree(&main_heap, buffer_start);
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
				kfree(&main_heap, buffer_start);
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
	kfree(&main_heap, buffer_start);
	return 0;
}

fs_node_t * iso9660_entry_to_node(iso9660_directory_entry_t * dir_entry, iso9660_fs * fs_info, fs_node_t * parent) {
	fs_node_mini_t miniVersion;
	miniVersion.inode = parent->inode;
	miniVersion.impl = parent->impl;
	miniVersion.recreate = parent->recreate;

	return iso9660_entry_to_node_mini(dir_entry, fs_info, miniVersion);
}

fs_node_t * iso9660_entry_to_node_mini(iso9660_directory_entry_t * dir_entry, iso9660_fs * fs_info, fs_node_mini_t parent) {
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
	response->impl = (uint32_t) fs_info;

	response->recreate = &iso9660_recreate;

	response->parent.inode = parent.inode;
	response->parent.impl = parent.impl;
	response->parent.recreate = parent.recreate;

	response->read = &iso9660_read;
	response->write = &iso9660_write;
	response->open = &iso9660_open;
	response->close = &iso9660_close;

	response->readdir = &iso9660_readdir;
	response->finddir = &iso9660_finddir;

	response->recreate = &iso9660_recreate;
	response->free_node = &iso9660_free_node;

	return response;
}

fs_node_t * iso9660_recreate(fs_node_mini_t * mini) {
	fs_node_t * response = kalloc(&main_heap, sizeof(fs_node_t));
	iso9660_fs * fs_info = (iso9660_fs *) mini->impl;

	void * entry_buffer = kalloc(&main_heap, 2048);
	void * entry_buffer_start = entry_buffer;
	((read_block_t)fs_info->dev->read_block)(fs_info->dev, entry_buffer, 1, mini->inode);

	void * table_buffer = kalloc(&main_heap, 2048);
	void * table_buffer_start = table_buffer;
	((read_block_t)fs_info->dev->read_block)(fs_info->dev, table_buffer, 1, ISO9660_OFFSET);

	iso9660_primary_volume_descriptor_t * vol_desc = (iso9660_primary_volume_descriptor_t *) table_buffer;
	((read_block_t)fs_info->dev->read_block)(fs_info->dev, table_buffer, 1, vol_desc->location_of_typel_path_table); // TODO: endianness

	iso9660_directory_entry_t * dir_entry = entry_buffer;
	iso9660_path_table_entry_t * path_entry = table_buffer;
	bool is_first_entry = true;

	while (path_entry->lba != mini->inode) {
		is_first_entry = false;
		if (path_entry->length_of_directory_identifier == 0) {
			kfree(&main_heap, table_buffer_start);
			kfree(&main_heap, entry_buffer_start);
			return 0; // no such lba, so let's not loop forever and read random junk
		}
		uint32_t entry_len = 8;
		entry_len += path_entry->length_of_directory_identifier;
		if (path_entry->length_of_directory_identifier % 2 == 1) {
			entry_len += 1;
		}

		// go to the next entry
		table_buffer += entry_len;
		path_entry = (iso9660_path_table_entry_t *) table_buffer;
	}

	uint32_t parent_num = path_entry->parent_dir_num;

	table_buffer = table_buffer_start;
	path_entry = (iso9660_path_table_entry_t *) table_buffer;

	fs_node_mini_t parent_mini;

	if (!is_first_entry) {
		for (uint32_t i = 1; i < parent_num; i++) {
			if (path_entry->length_of_directory_identifier == 0) {
				kfree(&main_heap, table_buffer_start);
				kfree(&main_heap, entry_buffer_start);
				return 0; // no such lba, so let's not loop forever and read random junk
			}
			uint32_t entry_len = 8;
			entry_len += path_entry->length_of_directory_identifier;
			if (path_entry->length_of_directory_identifier % 2 == 1) {
				entry_len += 1;
			}

			// go to the next entry
			table_buffer += entry_len;
			path_entry = (iso9660_path_table_entry_t *) table_buffer;
		}

		parent_mini.inode = path_entry->lba;
		parent_mini.impl = (uint32_t) fs_info;
		parent_mini.recreate = &iso9660_recreate;
	} else {
		// it's the root
		parent_mini.inode = fs_info->mountpoint.inode;
		parent_mini.impl = fs_info->mountpoint.impl;
		parent_mini.recreate = fs_info->mountpoint.recreate;
	}

	kfree(&main_heap, table_buffer_start);
	kfree(&main_heap, entry_buffer_start);
	return iso9660_entry_to_node_mini(dir_entry, fs_info, parent_mini);
}

void iso9660_free_node(fs_node_t * node) {
	kfree(&main_heap, node);
}

fs_node_t * iso9660_finddir(fs_node_t * current_dir, char * name) {
	//printf("iso9660: finddir: lba of current: %d\n", current_dir->inode);
	void * buffer = kalloc(&main_heap, 2048);
	void * buffer_start = buffer;
	iso9660_fs * fs_info = (iso9660_fs *) current_dir->impl;
	((read_block_t)fs_info->dev->read_block)(fs_info->dev, buffer, 1, current_dir->inode);

	iso9660_directory_entry_t * dir_entry = (iso9660_directory_entry_t *) buffer;
	bool skip_first = true;
	uint32_t count = 0;

	while (1) {
		if (dir_entry->length_of_record == 0) {
			kfree(&main_heap, buffer_start);
			return 0;
		}
		if (!skip_first && dir_entry->file_identifier_and_system_use[0] != '\1') { // why are these things
			if (dir_entry->length_of_file_identifier <= 255) {
				dir_entry->file_identifier_and_system_use[dir_entry->length_of_file_identifier] = '\0';
				if (!strcmp(dir_entry->file_identifier_and_system_use, name)) { // is it the right file?
					// yay, let's fill in a response and send it back
					fs_node_t * response = iso9660_entry_to_node(dir_entry, fs_info, current_dir);
					kfree(&main_heap, buffer_start);
					return response;
				}
			} else {
				// blegh no buffer overflow
				dbgprint("iso9660: warning: file identifier too large!\n");
				kfree(&main_heap, buffer_start);
				return 0;
			}
		}
		if (skip_first) {
			skip_first = false;
		}
		buffer += dir_entry->length_of_record;
		dir_entry = (iso9660_directory_entry_t *) buffer;
	}

	kfree(&main_heap, buffer_start);
	return 0;
}

void iso9660_open(fs_node_t *node, uint8_t read, uint8_t write) {

}

void iso9660_close(fs_node_t *node) {

}

uint32_t iso9660_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
	iso9660_fs * fs_info = (iso9660_fs *) node->impl;
	uint32_t readsize = size;
	if (readsize > node->length) {
		readsize = node->length;
	}
	//fs_info->dev
}

uint32_t iso9660_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
	return 0;
}
