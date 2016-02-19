#ifndef _KERNEL_VFS_H
#define _KERNEL_VFS_H

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08 // Is the file an active mountpoint?

typedef struct fs_node fs_node_t;

typedef uint32_t (*read_type_t)(fs_node_t*,uint32_t,uint32_t,uint8_t*);
typedef uint32_t (*write_type_t)(fs_node_t*,uint32_t,uint32_t,uint8_t*);
typedef void (*open_type_t)(fs_node_t*, uint8_t read, uint8_t write);
typedef void (*close_type_t)(fs_node_t*);
typedef struct dirent * (*readdir_type_t)(struct fs_node_mini*,uint32_t);
typedef struct fs_node * (*finddir_type_t)(fs_node_t*,char *name);

typedef struct fs_node * (*recreate_type_t)(struct fs_node_mini *);
typedef void (*free_node_type_t)(struct fs_node *);

// fs_node_mini_t is a reduced version of fs_node_t
// used to store information about parent directories
typedef struct {
	uint32_t inode;
	uint32_t impl;
	recreate_type_t recreate;
} __attribute__((packed)) fs_node_mini_t;

typedef struct fs_node {
	char name[256];			// The filename.
	uint32_t mask;			// The permissions mask.
	uint32_t uid;			// The owning user.
	uint32_t gid;			// The owning group.
	uint32_t flags;			// Includes the node type. See #defines above.
	uint32_t inode;			// This is device-specific - provides a way for a filesystem to identify files.
	uint32_t length;		// Size of the file, in bytes.
	uint32_t impl;			// An implementation-defined number.
	read_type_t read;
	write_type_t write;
	open_type_t open;
	close_type_t close;
	readdir_type_t readdir;
	finddir_type_t finddir;
	recreate_type_t recreate;
	free_node_type_t free_node;
	fs_node_mini_t parent;
	struct fs_node *ptr; // Used by mountpoints and symlinks.
} fs_node_t;

typedef struct { // One of these is returned by the readdir call, according to POSIX.
	uint32_t d_ino;

	char d_name[256];
} dirent;

extern fs_node_t fs_root; // The root of the filesystem.
extern fs_node_t fs_mnt; // storing mounted stuff
extern fs_node_t fs_cdrom_mnt;
extern fs_node_mini_t mini_root;

// Standard read/write/open/close functions. Note that these are all suffixed with
// _fs to distinguish them from the read/write/open/close which deal with file descriptors
// not file nodes.
uint32_t read_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t write_fs(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
void open_fs(fs_node_t *node, uint8_t read, uint8_t write);
void close_fs(fs_node_t *node);
dirent *readdir_fs(fs_node_t *node, uint32_t index);
fs_node_t *finddir_fs(fs_node_t *node, char *name);
void free_node_fs(fs_node_t *node);

fs_node_t * root_recreate(fs_node_mini_t * mini);

#endif
