#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/mem.h>
#include <kernel/panic.h>

#include <kernel/io.h>
#include <kernel/tty.h>
#include <kernel/vga.h>
#include <kernel/ps2keyboard.h>

void kernel_early(void)
{
	terminal_initialize();

	if (!mem_init()) {
		panic("Failed to load memory manager");
	}
}

void kernel_main(void)
{
	printf("Welcome to HexOS\n");

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

	while (1) {
		char in = getchar();
		//printf("%c", in);
		for (int i; i < 999999999; i++) {
			// this is my sleep()
			// for now
		}
	}
}
