#ifndef _KERNEL_ISO9660_H
#define _KERNEL_ISO9660_H

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
	// TODO: Volume Space Size and continue
} __attribute__((packed)) iso9660_primary_volume_descriptor_t;

#endif
