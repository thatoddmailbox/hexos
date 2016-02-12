#ifndef _KERNEL_PCI_H
#define _KERNEL_PCI_H

#include <stddef.h>
#include <stdint.h>

uint16_t pci_config_read_word (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

uint16_t pci_get_vendor(uint16_t bus, uint16_t device, uint16_t function);
uint16_t pci_get_device(uint16_t bus, uint16_t device, uint16_t function);

void pci_check_device(uint8_t bus, uint8_t device);
void pci_check_function(uint8_t bus, uint8_t device, uint8_t function);
uint16_t pci_check_vendor(uint8_t bus, uint8_t slot);

void pci_check_all_buses();

typedef struct {
	uint32_t vendor;
	uint32_t device;
	uint32_t func;
} pci_device;

#endif
