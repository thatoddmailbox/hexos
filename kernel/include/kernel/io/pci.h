#ifndef _KERNEL_PCI_H
#define _KERNEL_PCI_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
	uint32_t bus;
	uint32_t slot;
	uint32_t vendor;
	uint32_t device;
	uint32_t func;
	uint32_t class;
	uint32_t subclass;
} pci_device;

uint16_t pci_config_read_word (uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);

uint16_t pci_get_vendor(uint16_t bus, uint16_t device, uint16_t function);
uint16_t pci_get_device(uint16_t bus, uint16_t device, uint16_t function);
uint16_t pci_get_class(uint16_t bus, uint16_t device, uint16_t function);
uint16_t pci_get_subclass(uint16_t bus, uint16_t device, uint16_t function);

void pci_check_device(uint8_t bus, uint8_t device);
void pci_check_function(pci_device * dev);
uint16_t pci_check_vendor(uint8_t bus, uint8_t slot);

void pci_check_all_buses();

#endif
