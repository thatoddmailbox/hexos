#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <multiboot.h>

#include <kernel/blockdevice.h>

#include <kernel/clock.h>

#include <kernel/gdt.h>

#include <kernel/heap.h>

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

#include <kernel/vfs.h>

void kernel_early(unsigned long magic, multiboot_info_t* mb_info, uint32_t initial_stack) {
	gdt_install();

	terminal_initialize();

	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		panic("HexOS requires a Multiboot compliant bootloader.");
	}

	printf("cmdline = %s\n", mb_info->cmdline);

	if (!mem_init(mb_info)) {
		panic("Failed to load memory manager");
	}

	main_heap_init();

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

	//pic_init();

	// and enable interrupts!
	enable_interrupts();

	outb(0x70, inb(0x70)&0x7F); // enable non-maskable interrupts

	timer_install(); // install timer

	rtc_init(); // rtc setup

	pic_init(32,40);

	for(int i=0;i<16;i++) {
		pic_disable(i);
		pic_acknowledge(i);
	}

	clock_init();

	printf("ready to init process!");

	process_init();

	vfs_init();

	ata_init();

	keyboard_install(); // keyboard setup

	shell_init();

	dbgprint("Starting HexOS...\n");
}

void kernel_main(void) {
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

	/*printf("atapi_read: %d\n", atapi_read(2, buffer, 5, 0x10));
	printf("%s\n", (char*) buffer);

	printf("hex: %s\n", (char*) (buffer+702));

	printf("hex: %s\n", (char*) (buffer+2048));*/

	// TODO: make this less hardcoded
	// TODO: actually free memory and stuff
	ata_metadata metadata;
	metadata.ata_number = 2;

	block_device cdrom;
	cdrom.read_block = atapi_block_read;
	cdrom.metadata = &metadata;

	fs_node_mini_t mnt_mini;
	mnt_mini.impl = 0;
	mnt_mini.inode = 1;
	mnt_mini.recreate = &root_recreate;

	iso9660_init_volume(&cdrom, &fs_cdrom_mnt, mnt_mini);

	enable_interrupts();

	shell_prompt();

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
