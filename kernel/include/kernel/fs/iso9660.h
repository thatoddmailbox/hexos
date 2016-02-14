#ifndef _KERNEL_ISO9660_H
#define _KERNEL_ISO9660_H

#include <kernel/vfs.h>

#define ISO9600_OFFSET 0x10

#define ISO9600_BOOT_RECORD 0
#define ISO9600_PRIMARY_VOLUME 1
#define ISO9600_SUPPLEMENTARY_VOLUME 2
#define ISO9600_VOLUME_PARTITION 3
// ... reserved ...
#define ISO9600_VOLUME_DESC_SET_TERMINATOR 255

typedef struct {
	uint8_t type_code;
	char identifier[5]; // always 'cd001'
	uint8_t version;
	char data[2041];
} __attribute__((packed)) iso9660_volume_descriptor_t;

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

#endif
