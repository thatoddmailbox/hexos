#ifndef _KERNEL_ISO9660_H
#define _KERNEL_ISO9660_H

#include <kernel/vfs.h>

#define ISO9660_OFFSET 0x10

#define ISO9660_BOOT_RECORD 0
#define ISO9660_PRIMARY_VOLUME 1
#define ISO9660_SUPPLEMENTARY_VOLUME 2
#define ISO9660_VOLUME_PARTITION 3
// ... reserved ...
#define ISO9660_VOLUME_DESC_SET_TERMINATOR 255

#define ISO9660_FLAG_HIDDEN 0x01
#define ISO9660_FLAG_DIRECTORY 0x02
#define ISO9660_FLAG_ASSOCIATED 0x04
#define ISO9660_FLAG_E_ATTR_RECORD_HAS_INFO 0x08
#define ISO9660_FLAG_E_ATTR_RECORD_HAS_PERMS 0x10
#define ISO9660_FLAG_RESERVED_1 0x20
#define ISO9660_FLAG_RESERVED_2 0x40
#define ISO9660_FLAG_NOT_FINAL 0x80

typedef struct {
	uint8_t type_code;
	char identifier[5]; // always 'cd001'
	uint8_t version;
	char data[2041];
} __attribute__((packed)) iso9660_volume_descriptor_t;

// this date struct is only used in directory entries, but not in the primary volume descriptor
typedef struct {
	uint8_t years_since_1900;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	uint8_t offset_from_gmt; // in 15 min. intervals from -48 (West) to +52 (East)
} __attribute__((packed)) iso9660_date_t;

typedef struct {
	uint8_t length_of_directory_identifier;
	uint8_t extended_attr_record_length;
	uint32_t lba;
	uint16_t parent_dir_num;
	char directory_identifier[247];
} __attribute__((packed)) iso9660_path_table_entry_t;

typedef struct {
	uint8_t type;
	char identifier[5]; // always 'cd001'
	uint8_t version;
	char boot_system_identifier[32]; // strA
	char boot_identifier[32]; // strA
	char boot_system_use[1977];
} __attribute__((packed)) iso9660_boot_record_t;

typedef struct {
	uint8_t type_code;
	char identifier[5]; // always 'cd001'
	uint8_t version;
	uint8_t unused_1; // unused, should be 0
	char system_identifier[32]; // strA
	char volume_identifier[32]; // strD
	char unused_2[8];
	uint32_t volume_space_size; // TODO: THIS RELIES ON LITTLE ENDIAN HOST!!!
	uint32_t unused_3;
	char unused_4[32];
	uint16_t volume_set_size; // TODO: little endian only!
	uint16_t unused_5;
	uint16_t volume_seq_number; // TODO: little ENDIAN
	uint16_t unused_6;
	uint16_t logical_block_size; // TODO: LITTLE endian
	uint16_t unused_7;
	uint32_t path_table_size; // TODO: LITTLE ENDIAN!
	uint32_t unused_8;
	uint32_t location_of_typel_path_table;
	uint32_t location_of_optional_typel_path_table;
	uint32_t location_of_typem_path_table;
	uint32_t location_of_optional_typem_path_table;
	char directory_entry[34]; // TODO: make type for this
	char volume_set_identifier[128];
	char publisher_identifier[128];
	char data_preparer_identifier[128];
	char application_identifier[128];
	char copyright_file_identifier[38];
	char abstract_file_identifier[38];
	char bibliographic_file_identifier[37];

	char volume_creation_date_and_time[17]; // TODO: make type for this
	char volume_modification_date_and_time[17]; // TODO: make type for this
	char volume_effective_date_and_time[17]; // TODO: make type for this

	uint8_t file_structure_version;
	uint8_t unused_9;
	char application_use[512];
	char reserved[653];
} __attribute__((packed)) iso9660_primary_volume_descriptor_t;

typedef struct {
	uint8_t length_of_record;
	uint8_t extended_attr_record_length;

	uint32_t lba; // TODO: endianness
	uint32_t unused_1;

	uint32_t length_of_file; // TODO: endianness
	uint32_t unused_2;

	iso9660_date_t recording_date;

	uint8_t flags;
	uint8_t interleaved_unit_size;
	uint8_t interleaved_gap_size;
	uint16_t volume_sequence_number; // TODO: endianness
	uint16_t unused_3;
	uint8_t length_of_file_identifier;
	char file_identifier_and_system_use[255]; // it's not really 255 bytes, but 255 is the max for the whole record, so
} __attribute__((packed)) iso9660_directory_entry_t;

typedef struct {
	block_device * dev;
	fs_node_mini_t mountpoint;
} iso9660_fs;

void iso9660_init_volume(block_device * dev, fs_node_t * target, fs_node_mini_t target_parent);

fs_node_t * iso9660_entry_to_node(iso9660_directory_entry_t * dir_entry, iso9660_fs * fs_info, fs_node_t * parent);
fs_node_t * iso9660_entry_to_node_mini(iso9660_directory_entry_t * dir_entry, iso9660_fs * fs_info, fs_node_mini_t parent);

fs_node_t * iso9660_recreate(fs_node_mini_t * mini);
void iso9660_free_node(fs_node_t * node);

dirent * iso9660_readdir(fs_node_t * this, uint32_t i);
fs_node_t * iso9660_finddir(fs_node_t * current_dir, char * name);

void iso9660_open(fs_node_t *node, uint8_t read, uint8_t write);
void iso9660_close(fs_node_t *node);
uint32_t iso9660_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t iso9660_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);

#endif
