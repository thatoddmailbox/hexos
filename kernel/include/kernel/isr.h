#ifndef _KERNEL_ISR_H
#define _KERNEL_ISR_H

#include <stddef.h>

void isrs_install();
void fault_handler(struct regs *r);

// IRQs things
void irq_install_handler(int irq, void (*handler)(struct regs *r));
void irq_uninstall_handler(int irq);
void irq_remap(void);
void irq_install();
void irq_handler(struct regs *r);

#endif
