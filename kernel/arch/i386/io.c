#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/io.h>

void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %[port], %[ret]"
                   : [ret] "=a"(ret)   // using symbolic operand names as an example, mainly because they're not used in order
                   : [port] "Nd"(port) );
    return ret;
}

void outl(uint16_t port, uint32_t val) {
    asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}

uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ( "inl %[port], %[ret]"
                   : [ret] "=a"(ret)   // using symbolic operand names as an example, mainly because they're not used in order
                   : [port] "Nd"(port) );
    return ret;
}

void io_wait(void) {
    // Port 0x80 is used for 'checkpoints' during POST.
    // The Linux kernel seems to think it is free for use :-/
    asm volatile ( "outb %%al, $0x80" : : "a"(0) );
    // %%al instead of %0 makes no difference.  TODO: does the register need to be zeroed?
}

// serial port stuff

#define PORT 0x3f8  /* COM1 */

void serial_init() {
   outb(PORT + 1, 0x00);    // Disable all interrupts
   outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
   outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
   outb(PORT + 1, 0x00);    //                  (hi byte)
   outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
   outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
   outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

int serial_is_transmit_empty() {
   return inb(PORT + 5) & 0x20;
}

void serial_write(char a) {
   while (serial_is_transmit_empty() == 0);

   outb(PORT,a);
}

int serial_received() {
   return inb(PORT + 5) & 1;
}

char serial_read() {
   while (serial_received() == 0);

   return inb(PORT);
}

void dbgprint(const char* data) {
	for ( size_t i = 0; i < strlen(data); i++ )
		serial_write((int) ((const unsigned char*) data)[i]);
}
