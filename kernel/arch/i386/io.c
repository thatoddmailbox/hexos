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

uint16_t inw(int port) {
	uint16_t result;
	asm("inw %w1, %w0" : "=a" (result) : "Nd" (port) );
	return result;
}

void outw(int port, uint16_t value) {
	asm("outw %w0, %w1" : : "a" (value), "Nd" (port) );
}

void io_wait(void) {
    outb(0x80, 0);
}

// slow versions
// they iowait()
uint8_t inb_slow(uint16_t port) {
    uint8_t ret = inb(port);
    io_wait();
    return ret;
}

void outb_slow(uint16_t port, uint8_t val) {
    outb(port, val);
    io_wait();
}

// serial port stuff

#define PORT 0x3f8  /* COM1 */

bool serial_inited = false;

void serial_init() {
    outb(PORT + 1, 0x00);    // Disable all interrupts
    outb(PORT + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(PORT + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT + 1, 0x00);    //                  (hi byte)
    outb(PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(PORT + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    serial_inited = true;
}

int serial_is_transmit_empty() {
    if (!serial_inited) {
        return true;
    }
    return inb(PORT + 5) & 0x20;
}

void serial_write(char a) {
    if (!serial_inited) {
        return;
    }
    while (serial_is_transmit_empty() == 0);

    outb(PORT,a);
}

int serial_received() {
    if (!serial_inited) {
        return 0;
    }
    return inb(PORT + 5) & 1;
}

char serial_read() {
    if (!serial_inited) {
        return 'e';
    }
    while (serial_received() == 0);

    return inb(PORT);
}

void dbgprint(const char* data) {
    if (!serial_inited) {
        return;
    }
    for ( size_t i = 0; i < strlen(data); i++ )
    serial_write((int) ((const unsigned char*) data)[i]);
}
