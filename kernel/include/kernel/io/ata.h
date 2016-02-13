#ifndef _KERNEL_ATA_H
#define _KERNEL_ATA_H

#include <stddef.h>
#include <stdint.h>

void ata_init();

void ata_reset( int unit );
int ata_probe( int unit, int *nblocks, int *blocksize, char *name );

int ata_read( int unit, void *buffer, int nblocks, int offset );
int ata_write( int unit, void *buffer, int nblocks, int offset );
int atapi_read( int unit, void *buffer, int nblocks, int offset );

#endif
