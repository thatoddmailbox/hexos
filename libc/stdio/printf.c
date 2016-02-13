#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

void print(const char* data, size_t data_length) {
	for ( size_t i = 0; i < data_length; i++ )
		putchar((int) ((const unsigned char*) data)[i]);
}

void printf_int(int i) {
	if (i < 0) { // if it's negative
		putchar('-'); // put a negative sign
		i *= -1; // and make it positive
	}

	// this code is modified from basekernel, which is under the GPL
	// see https://github.com/dthain/basekernel/blob/6fff9df12906787b16ba94d685c3ec5bf28eb1eb/src/string.c
	int f, d;
	f = 1;
	while((i/f)>0) {
		f *= 10;
	}
	f = f/10;
	if (f == 0) {
		f=1;
	}
	while (f>0) {
		d = i/f;
		putchar('0'+d);
		i = i-d*f;
		f = f/10;
	}
}

// printf_hexdigit and printf_hex are modified from basekernel, which is under the GPL
// see https://github.com/dthain/basekernel/blob/6fff9df12906787b16ba94d685c3ec5bf28eb1eb/src/string.c
void printf_hexdigit(unsigned char i)
{
	if(i<10) {
		putchar('0'+i);
	} else {
		putchar('a'+i-10);
	}
}

void printf_hex(unsigned int i)
{
	int j;
	for(j=28;j>=0;j=j-4) {
		printf_hexdigit((i>>j)&0x0f);
	}
}

int printf(const char* restrict format, ...) {
	va_list parameters;
	va_start(parameters, format);

	int written = 0;
	size_t amount;
	bool rejected_bad_specifier = false;

	while ( *format != '\0' )
	{
		if ( *format != '%' )
		{
		print_c:
			amount = 1;
			while ( format[amount] && format[amount] != '%' )
				amount++;
			print(format, amount);
			format += amount;
			written += amount;
			continue;
		}

		const char* format_begun_at = format;

		if ( *(++format) == '%' )
			goto print_c;

		if ( rejected_bad_specifier )
		{
		incomprehensible_conversion:
			rejected_bad_specifier = true;
			format = format_begun_at;
			goto print_c;
		}

		if ( *format == 'c' )
		{
			format++;
			char c = (char) va_arg(parameters, int /* char promotes to int */);
			print(&c, sizeof(c));
		}
		else if ( *format == 's' )
		{
			format++;
			const char* s = va_arg(parameters, const char*);
			print(s, strlen(s));
		}
		else if ( *format == 'd' )
		{
			format++;
			int s = va_arg(parameters, int);
			printf_int(s);
		}
		else if ( *format == 'x' )
		{
			format++;
			unsigned int s = va_arg(parameters, unsigned int);
			printf_hex(s);
		}
		else
		{
			goto incomprehensible_conversion;
		}
	}

	va_end(parameters);

	return written;
}
