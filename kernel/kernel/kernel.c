#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/io.h>

#include <kernel/tty.h>
#include <kernel/ps2keyboard.h>

void kernel_early(void)
{
	terminal_initialize();
}

void kernel_main(void)
{
	printf("Welcome to HexOS\n");
	while (1) {
		printf("%c", getchar());
	}
}
