#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <kernel/vga.h>

__attribute__((__noreturn__))
void panic(char * msg)
{
	// TODO: make this not rely on terminal_*

	terminal_setcolor(COLOR_RED);
	printf("A fatal error has occurred and HexOS has been shut down.\nMessage: ");
	printf(msg);
	printf("\n");
	while ( 1 ) { }
	__builtin_unreachable();
}
