#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <multiboot.h>

#include <kernel/clock.h>

#include <kernel/idt.h>
#include <kernel/interrupts.h>
#include <kernel/isr.h>

#include <kernel/mem.h>
#include <kernel/panic.h>

#include <kernel/cpuid.h>
#include <kernel/timer.h>

#include <kernel/io.h>
#include <kernel/tty.h>

#include <kernel/io/ata.h>
#include <kernel/io/pci.h>

#include <kernel/process.h>

#include <kernel/rtc.h>

#include <kernel/vga.h>
#include <kernel/ps2keyboard.h>

void kernel_early(unsigned long magic, multiboot_info_t* mb_info, uint32_t initial_stack)
{
	gdt_install();

	terminal_initialize();

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		panic("HexOS requires a Multiboot compliant bootloader.");
	}

	printf("cmdline = %s\n", mb_info->cmdline);

	if (!mem_init(mb_info)) {
		panic("Failed to load memory manager");
	}

	pci_install(); // pci stuff

	// TODO: write a strcmp() and a proper args parser
	if (((char*)mb_info->cmdline)[0] == '-' &&
		((char*)mb_info->cmdline)[1] == 's' &&
		((char*)mb_info->cmdline)[2] == 'e' &&
		((char*)mb_info->cmdline)[3] == 'r' &&
		((char*)mb_info->cmdline)[4] == 'd' &&
		((char*)mb_info->cmdline)[5] == 'b' &&
		((char*)mb_info->cmdline)[6] == 'g') {
		serial_init(); // enable serial if the right option selected
	}
	dbgprint("Hello!\n");

	cpu_brand_name();

	dbgprint("Setting up interrupts...\n");
	idt_install(); // set up the table
	isrs_install(); // set up the isrs
	irq_install(); // set up the irqs

	pic_init();

	// and enable interrupts!
	enable_interrupts();

	outb(0x70, inb(0x70)&0x7F); // enable non-maskable interrupts

	timer_install(); // install timer

	keyboard_install(); // keyboard setup

	rtc_init(); // rtc setup

	pic_init(32,40);

	for(int i=0;i<16;i++) {
		pic_disable(i);
		pic_acknowledge(i);
	}

	clock_init();

	process_init();

	ata_init();

	dbgprint("Starting HexOS...\n");
}

void kernel_main(void)
{
	printf("Welcome to HexOS\n");
	dbgprint("Welcome to HexOS\n");

	//beep();

	terminal_setcolor(COLOR_GREEN);
	printf("    _   _            ___  ____   \n");
	printf("   | | | | _____  __/ _ \\/ ___|  \n");
	printf("   | |_| |/ _ \\ \\/ / | | \\___ \\  \n");
	printf("   |  _  |  __/>  <| |_| |___) | \n");
	printf("   |_| |_|\\___/_/\\_\\___/|____/  \n");
	terminal_setcolor(COLOR_WHITE);

	/*char * test = hex_malloc(11);
	test[0] = 'a';
	test[1] = '\0';

	char * test2 = hex_malloc(11);
	test2[0] = 'b';
	test2[1] = '\0';*/

	char input = ' ';
	char lastChar = ' ';
	while (1) {
		while (serial_received()) {
			input = serial_read();
			serial_write(input);
			printf("%c", input);
		}
	}
}
