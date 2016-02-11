#include <kernel/idt.h>
#include <kernel/isr.h>
#include <kernel/timer.h>

unsigned int timer_ticks = 0;

unsigned int getTicks() {
	return timer_ticks;
}

void sleep(int toWait) {
	int started = getTicks();
	while (started - getTicks() < (toWait * 18)) {
		__asm__ __volatile__ ("nop");
	}
}

void timer_handler(struct regs *r) {
	timer_ticks++;
    /* Every 18 clocks (approximately 1 second), we will
    *  display a message on the screen */
    if (timer_ticks % 18 == 0)
    {
        dbgprint("One second has passed\n");
    }
}

void timer_install() {
	irq_install_handler(0, timer_handler);
}
