#include <stddef.h>

#include <stdbool.h>
#include <stdint.h>

#include <kernel/io/pci.h>

pci_device pci_devices[32];

uint16_t pci_config_read_word(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address;
    uint32_t lbus  = (uint32_t)bus;
    uint32_t lslot = (uint32_t)slot;
    uint32_t lfunc = (uint32_t)func;
    uint16_t tmp = 0;

    address = (uint32_t)((lbus << 16) | (lslot << 11) |
    (lfunc << 8) | (offset & 0xfc) | ((uint32_t)0x80000000));

    // write out the address
    outl(0xCF8, address);
    // read in the data
    // (offset & 2) * 8) = 0 will choose the first word of the 32 bits register
    tmp = (uint16_t)((inl(0xCFC) >> ((offset & 2) * 8)) & 0xffff);
    return (tmp);
}

uint16_t pci_get_vendor(uint16_t bus, uint16_t device, uint16_t function) {
    uint32_t r0 = pci_config_read_word(bus,device,function,0);
    return r0;
}

uint16_t pci_get_device(uint16_t bus, uint16_t device, uint16_t function) {
    uint32_t r0 = pci_config_read_word(bus,device,function,2);
    return r0;
}

uint16_t pci_check_vendor(uint8_t bus, uint8_t slot) {
    uint16_t vendor, device;
    /* try and read the first configuration register. Since there are no */
    /* vendors that == 0xFFFF, it must be a non-existent device. */
    if ((vendor = pci_config_read_word(bus,slot,0,0)) != 0xFFFF) {
        device = pci_config_read_word(bus,slot,0,2);
        printf("hexy_hex\n");
    }
    return (vendor);
}

void pci_check_device(uint8_t bus, uint8_t device) {
    uint8_t func = 0;

    uint16_t vendorID;

    vendorID = pci_get_vendor(bus, device, func);
    if (vendorID == 0xFFFF) return;        // Device doesn't exist
    pci_check_function(bus, device, func);
    /*(headerType = getHeaderType(bus, device, func);
    if( (headerType & 0x80) != 0) {
        // It is a multi-function device, so check remaining functions
        for(func = 1; func < 8; func++) {
            if(getVendorID(bus, device, func) != 0xFFFF) {
                pci_check_function(bus, device, func);
            }
        }
    }*/
}

void pci_check_function(uint8_t bus, uint8_t device, uint8_t function) {
    /*printf("PCI device: bus=0x");

    char buf[10] = {0};
    itoa(bus, buf, 16);
    printf(buf);
    printf(", device=0x");

    buf[0] = 0; buf[1] = 0; buf[2] = 0;	buf[3] = 0;	buf[4] = 0;	buf[5] = 0;	buf[6] = 0;	buf[7] = 0;	buf[8] = 0;	buf[9] = 0;
    itoa(device, buf, 16);
    printf(buf);
    printf(", function=0x");

    buf[0] = 0; buf[1] = 0; buf[2] = 0;	buf[3] = 0;	buf[4] = 0;	buf[5] = 0;	buf[6] = 0;	buf[7] = 0;	buf[8] = 0;	buf[9] = 0;
    itoa(function, buf, 16);
    printf(buf);

    printf("\n");*/
}

void pci_check_all_buses() {
    int i = 0;
    for(uint32_t bus = 0; bus < 256; bus++) {
        for(uint32_t slot = 0; slot < 32; slot++) {
            for(uint32_t function = 0; function < 8; function++) {
                uint16_t vendor = pci_get_vendor(bus, slot, function);
                if(vendor == 0xffff) continue;
                uint16_t device = pci_get_device(bus, slot, function);
                pci_check_function(bus, device, function);

                /*printf("vendor=");

                char buf[10] = {0};
                itoa(vendor, buf, 16);
                printf(buf);
                printf("\n");*/

                pci_devices[i].vendor = vendor;
                pci_devices[i].device = device;
                pci_devices[i].func = function;
                i++;
            }
        }
    }
    char buf[3] = {0};
    itoa(i, buf, 10);
    printf("Found %s PCI devices!\n", buf);
}

void pci_install() {
    pci_check_all_buses();
}
