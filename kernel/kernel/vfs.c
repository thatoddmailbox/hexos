#include <kernel/mem.h>
#include <kernel/vfs.h>
#include <string.h>

fs_node_t fs_root;
fs_node_t fs_mnt;

uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
	// Has the node got a read callback?
	if (node->read != 0) {
		return node->read(node, offset, size, buffer);
	} else {
		return 0;
	}
}

uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
	// Has the node got a write callback?
	if (node->write != 0) {
		return node->write(node, offset, size, buffer);
	} else {
		return 0;
	}
}

void open_fs(fs_node_t *node, uint8_t read, uint8_t write) {
	// Has the node got an open callback?
	if (node->open != 0) {
		return node->open(node, read, write);
	}
}

void close_fs(fs_node_t *node) {
	// Has the node got a close callback?
	if (node->close != 0) {
		return node->close(node);
	}
}

dirent *readdir_fs(fs_node_t *node, uint32_t index) {
	if ((node->flags&0x7) == FS_DIRECTORY && node->readdir != 0) {
		return node->readdir(node, index);
	} else {
		return 0;
	}
}

fs_node_t *finddir_fs(fs_node_t *node, char *name) {
	if ((node->flags&0x7) == FS_DIRECTORY && node->finddir != 0) {
		return node->finddir(node, name);
	} else {
		return 0;
	}
}

dirent dirent_resp;

dirent * root_readdir(fs_node_t* node, uint32_t num) {
	if (node == &fs_root) {
		if (num == 0) {
			strcpy(dirent_resp.d_name, "mnt");
			dirent_resp.d_name[3] = 0;
			dirent_resp.d_ino = 0;
			return &dirent_resp;
		}
	} else if (node == &fs_mnt) {
		if (num == 0) {
			strcpy(dirent_resp.d_name, "cdrom");
			dirent_resp.d_name[6] = 0;
			dirent_resp.d_ino = 0;
			return &dirent_resp;
		}
	}
	return 0;
	//return &dirent_resp;
}

fs_node_t * root_finddir(fs_node_t * node, char * name) {
	if (node == &fs_root) {
		if (!strcmp(name, "mnt")) {
			return &fs_mnt;
		}
	}
	return 0;
}

void vfs_init() {
	strcpy(fs_mnt.name, "mnt");
	fs_mnt.flags = FS_DIRECTORY;
	fs_mnt.parent = &fs_root;
	fs_mnt.readdir = &root_readdir;
	fs_mnt.finddir = &root_finddir;

	fs_root.flags = FS_DIRECTORY;
	fs_root.readdir = &root_readdir;
	fs_root.finddir = &root_finddir;
	fs_root.parent = 0;
}