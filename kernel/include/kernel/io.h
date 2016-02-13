#ifndef _KERNEL_IO_H
#define _KERNEL_IO_H

#include <stddef.h>
#include <stdint.h>

void outb(uint16_t port, uint8_t val);
uint8_t inb(uint16_t port);

void outl(uint16_t port, uint32_t val);
uint32_t inl(uint16_t port);

uint16_t inw(int port);
void outw(uint16_t value, int port);

void io_wait(void);

// serial ports
void serial_init();
int serial_is_transmit_empty();
void serial_write(char a);
int serial_received();
char serial_read();

void dbgprint(const char* data);

#endif
