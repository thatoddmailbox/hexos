#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <multiboot.h>

#include <kernel/idt.h>
#include <kernel/isr.h>

#include <kernel/mem.h>
#include <kernel/panic.h>

#include <kernel/cpuid.h>

#include <kernel/io.h>
#include <kernel/tty.h>

#include <kernel/vga.h>
#include <kernel/ps2keyboard.h>

bool asdf = false;

void kernel_early(unsigned long magic, multiboot_info_t* mb_info)
{
	gdt_install();

	terminal_initialize();

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		panic("HexOS requires a Multiboot compliant bootloader.");
	}

	if (!mem_init(mb_info)) {
		panic("Failed to load memory manager");
	}

	serial_init();

	cpu_brand_name();

	char thing[33] = {0};
	itoa(0xdeadbeef, thing, 16);
	dbgprint(thing);
	dbgprint("\n");

	idt_install(); // set up the table
	isrs_install(); // set up the isrs
	irq_install(); // set up the irqs

	// and enable interrupts!
	__asm__ __volatile__ ("sti");

	timer_install();

	dbgprint("Starting HexOS...\n");
}

void kernel_main(void)
{
	printf("Welcome to HexOS\n");
	dbgprint("Welcome to HexOS\n");

	terminal_setcolor(COLOR_GREEN);
	printf("    _   _            ___  ____   \n");
	printf("   | | | | _____  __/ _ \\/ ___|  \n");
	printf("   | |_| |/ _ \\ \\/ / | | \\___ \\  \n");
	printf("   |  _  |  __/>  <| |_| |___) | \n");
	printf("   |_| |_|\\___/_/\\_\\___/|____/  \n");
	terminal_setcolor(COLOR_WHITE);

	printf("hexhexhex\n");

	char * test = hex_malloc(11);
	test[0] = 'a';
	test[1] = '\0';

	char * test2 = hex_malloc(11);
	test2[0] = 'b';
	test2[1] = '\0';

	char input = ' ';
	char lastChar = ' ';
	while (1) {
		while (serial_received()) {
			input = serial_read();
			serial_write(input);
			printf("%c", input);
		}
		input = getchar();
		if (input != lastChar) {
			serial_write(input);
			printf("%c", input);
			lastChar = input;
		}
	}
}
