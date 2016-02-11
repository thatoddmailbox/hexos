#ifndef _KERNEL_TIMER_H
#define _KERNEL_TIMER_H

unsigned int getTicks();
void sleep(int toWait);
void timer_handler(struct regs *r);
void timer_install();

#endif
