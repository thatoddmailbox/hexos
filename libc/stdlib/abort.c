#include <stdio.h>
#include <stdlib.h>

__attribute__((__noreturn__))
void abort(void)
{
	panic("abort() was called");
}
