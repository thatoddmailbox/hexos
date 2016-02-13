#ifndef _KERNEL_INTERRUPTS_H
#define _KERNEL_INTERRUPTS_H

extern void enable_interrupts();
extern void disable_interrupts();
extern void wait_interrupts();

extern void *interrupt_stack_pointer;

extern void intr_return();

#endif
