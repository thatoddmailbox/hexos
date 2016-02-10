#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <kernel/ps2keyboard.h>

char scancode[] = {
	'\e', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
	'[', ']', 'e', 'l', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 'l', '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm',
	',', '.', '/', 'r', '*', 'a', ' ', 'c', 'f', 'f', 'f', 'f', 'f', 'f', 'f', 'f', 'f', 'f', 'n', 's', '7', '8', '9', '-', '4',
	'5', '6', '+', '1', '2', '3', '0', '.', ' ', ' ', ' ', 'f', 'f'
};

char getScancode()
{
	char c=0;
	do {
		if(inb(0x60)!=c)
		{
			c=inb(0x60);
			if(c>0)
			return c;
		}
	} while(1);
}

char getchar()
{
	return scancode[getScancode()-1];
}
