#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/io.h>
#include <kernel/tty.h>
#include <kernel/vga.h>
#include <kernel/ps2keyboard.h>

void kernel_early(void)
{
	terminal_initialize();
}

void kernel_main(void)
{
	printf("Welcome to HexOS\n");

	terminal_setcolor(COLOR_GREEN);
	printf("	_   _            ___  ____   \n");
	printf("   | | | | _____  __/ _ \\/ ___|  \n");
	printf("   | |_| |/ _ \\ \\/ / | | \\___ \\  \n");
	printf("   |  _  |  __/>  <| |_| |___) | \n");
	printf("   |_| |_|\\___/_/\\_\\___/|____/  \n");
	terminal_setcolor(COLOR_WHITE);

	while (1) {
		printf("%c", getchar());
	}
}
