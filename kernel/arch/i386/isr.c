#include <kernel/idt.h>
#include <kernel/interrupts.h>
#include <kernel/io.h>

#include <kernel/process.h>

#include <kernel/tty.h>
#include <kernel/panic.h>
#include <kernel/vga.h>

static interrupt_handler_t interrupt_handlers[32];

/* These are function prototypes for all of the exception
*  handlers: The first 32 entries in the IDT are reserved
*  by Intel, and are designed to service exceptions! */
extern void isr0();
extern void isr1();
extern void isr2();
extern void isr3();
extern void isr4();
extern void isr5();
extern void isr6();
extern void isr7();
extern void isr8();
extern void isr9();
extern void isr10();
extern void isr11();
extern void isr12();
extern void isr13();
extern void isr14();
extern void isr15();
extern void isr16();
extern void isr17();
extern void isr18();
extern void isr19();
extern void isr20();
extern void isr21();
extern void isr22();
extern void isr23();
extern void isr24();
extern void isr25();
extern void isr26();
extern void isr27();
extern void isr28();
extern void isr29();
extern void isr30();
extern void isr31();

/* Set the access flags to 0x8E. This means that the entry is present, is
*  running in ring 0 (kernel level), and has the lower 5 bits
*  set to the required '14', which is represented by 'E' in hex. */
void isrs_install()
{
    idt_set_gate(0, (unsigned)isr0, 0x08, 0x8E);
    idt_set_gate(1, (unsigned)isr1, 0x08, 0x8E);
    idt_set_gate(2, (unsigned)isr2, 0x08, 0x8E);
    idt_set_gate(3, (unsigned)isr3, 0x08, 0x8E);
    idt_set_gate(4, (unsigned)isr4, 0x08, 0x8E);
    idt_set_gate(5, (unsigned)isr5, 0x08, 0x8E);
    idt_set_gate(6, (unsigned)isr6, 0x08, 0x8E);
    idt_set_gate(7, (unsigned)isr7, 0x08, 0x8E);

    idt_set_gate(8, (unsigned)isr8, 0x08, 0x8E);
    idt_set_gate(9, (unsigned)isr9, 0x08, 0x8E);
    idt_set_gate(10, (unsigned)isr10, 0x08, 0x8E);
    idt_set_gate(11, (unsigned)isr11, 0x08, 0x8E);
    idt_set_gate(12, (unsigned)isr12, 0x08, 0x8E);
    idt_set_gate(13, (unsigned)isr13, 0x08, 0x8E);
    idt_set_gate(14, (unsigned)isr14, 0x08, 0x8E);
    idt_set_gate(15, (unsigned)isr15, 0x08, 0x8E);

    idt_set_gate(16, (unsigned)isr16, 0x08, 0x8E);
    idt_set_gate(17, (unsigned)isr17, 0x08, 0x8E);
    idt_set_gate(18, (unsigned)isr18, 0x08, 0x8E);
    idt_set_gate(19, (unsigned)isr19, 0x08, 0x8E);
    idt_set_gate(20, (unsigned)isr20, 0x08, 0x8E);
    idt_set_gate(21, (unsigned)isr21, 0x08, 0x8E);
    idt_set_gate(22, (unsigned)isr22, 0x08, 0x8E);
    idt_set_gate(23, (unsigned)isr23, 0x08, 0x8E);

    idt_set_gate(24, (unsigned)isr24, 0x08, 0x8E);
    idt_set_gate(25, (unsigned)isr25, 0x08, 0x8E);
    idt_set_gate(26, (unsigned)isr26, 0x08, 0x8E);
    idt_set_gate(27, (unsigned)isr27, 0x08, 0x8E);
    idt_set_gate(28, (unsigned)isr28, 0x08, 0x8E);
    idt_set_gate(29, (unsigned)isr29, 0x08, 0x8E);
    idt_set_gate(30, (unsigned)isr30, 0x08, 0x8E);
    idt_set_gate(31, (unsigned)isr31, 0x08, 0x8E);
}

/* This is a simple string array. It contains the message that
*  corresponds to each and every exception. We get the correct
*  message by accessing like:
*  exception_message[interrupt_number] */
const char *exception_messages[] =
{
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",

    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Security Exception",
    "Reserved"
};

void interrupt_register(int i, interrupt_handler_t handler)
{
    interrupt_handlers[i] = handler;
}

// This function handles all interrupts.
void fault_handler(struct regs *r)
{
    dbgprint("interrupt!\n");
    if (interrupt_handlers[r->int_no] != 0) {
        (interrupt_handlers[r->int_no]) (r->int_no, r->int_no); // not sure what the second param is for...
        return;
    }
    // unhandled!
    // Is this a fault whose number is from 0 to 31?
    if (r->int_no < 32)
    {
        if (!current) {
            if (r->int_no == 2) {
                // it's an NMI. oh noes
                // we shouldn't do much other than text, to avoid triggering the faulty hardware again

                terminal_initialize(); // clear terminal
                terminal_setcolor(COLOR_RED); // set it to a spooky color

                // show scary message for scary error
                terminal_writestring("*** NON-MASKABLE INTERRUPT OCCURRED ***\n\n");

                terminal_writestring("To protect your computer and data, HexOS has stopped.\n\n");

                terminal_writestring("A critical non-recoverable and non-maskable hardware interrupt has occurred. This may mean that your computer's hardware is defective. If this error occurs multiple times, try isolating the problem by removing and/or replacing components.");
                while (1) {}
            }
            printf("Error code: %d\n", r->err_code);
            // Display the description for the Exception that occurred.
            panic(exception_messages[r->int_no]);
            for (;;);
        } else {
            // TODO: signal the process rather than kill it
            process_exit(-1, exception_messages[r->int_no]); // kill the process
        }
    }
}

/* These are own ISRs that point to our special IRQ handler
*  instead of the regular 'fault_handler' function */
extern void irq0();
extern void irq1();
extern void irq2();
extern void irq3();
extern void irq4();
extern void irq5();
extern void irq6();
extern void irq7();
extern void irq8();
extern void irq9();
extern void irq10();
extern void irq11();
extern void irq12();
extern void irq13();
extern void irq14();
extern void irq15();

/* This array is actually an array of function pointers. We use
*  this to handle custom IRQ handlers for a given IRQ */
void *irq_routines[16] =
{
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

/* This installs a custom IRQ handler for the given IRQ */
void irq_install_handler(int irq, void (*handler)(struct regs *r))
{
    irq_routines[irq] = handler;
}

/* This clears the handler for a given IRQ */
void irq_uninstall_handler(int irq)
{
    irq_routines[irq] = 0;
}

void irq_remap(void)
{
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0x0);
    outb(0xA1, 0x0);
}

void irq_install()
{
    irq_remap();

    idt_set_gate(32, (unsigned)irq0, 0x08, 0x8E);
    idt_set_gate(33, (unsigned)irq1, 0x08, 0x8E);
    idt_set_gate(34, (unsigned)irq2, 0x08, 0x8E);
    idt_set_gate(35, (unsigned)irq3, 0x08, 0x8E);
    idt_set_gate(36, (unsigned)irq4, 0x08, 0x8E);
    idt_set_gate(37, (unsigned)irq5, 0x08, 0x8E);
    idt_set_gate(38, (unsigned)irq6, 0x08, 0x8E);
    idt_set_gate(39, (unsigned)irq7, 0x08, 0x8E);
    idt_set_gate(40, (unsigned)irq8, 0x08, 0x8E);
    idt_set_gate(41, (unsigned)irq9, 0x08, 0x8E);
    idt_set_gate(42, (unsigned)irq10, 0x08, 0x8E);
    idt_set_gate(43, (unsigned)irq11, 0x08, 0x8E);
    idt_set_gate(44, (unsigned)irq12, 0x08, 0x8E);
    idt_set_gate(45, (unsigned)irq13, 0x08, 0x8E);
    idt_set_gate(46, (unsigned)irq14, 0x08, 0x8E);
    idt_set_gate(47, (unsigned)irq15, 0x08, 0x8E);
}

#define PIC_ICW1 0x11
#define PIC_ICW4_MASTER 0x01
#define PIC_ICW4_SLAVE  0x05
#define PIC_ACK_SPECIFIC 0x60

static uint8_t pic_control[2] = { 0x20, 0xa0 };
static uint8_t pic_data[2]    = { 0x21, 0xa1 };

void pic_init( int pic0base, int pic1base )
{
    outb(pic_control[0], PIC_ICW1);
    outb(pic_data[0], pic0base);
    outb(pic_data[0], 1<<2);
    outb(pic_data[0], PIC_ICW4_MASTER);
    outb(pic_data[0], ~(1<<2));

    outb(pic_control[1], PIC_ICW1);
    outb(pic_data[1], pic1base);
    outb(pic_data[1], 2);
    outb(pic_data[1], PIC_ICW4_SLAVE);
    outb(pic_data[1], ~0);

    printf("pic: ready\n");
}

void irq_enable(uint8_t irq)
{
    uint8_t mask;
    if(irq<8) {
        mask = inb(pic_data[0]);
        mask = mask&~(1<<irq);
        outb(pic_data[0], mask);
    } else {
        mask = inb(pic_data[1]);
        mask = mask&~(1<<irq);
        outb(pic_data[1], mask);
        irq_enable(2);
    }
}

void pic_disable(uint8_t irq) {
    uint8_t mask;
    if(irq<8) {
        mask = inb(pic_data[0]);
        mask = mask|(1<<irq);
        outb(pic_data[0], mask);
    } else {
        mask = inb(pic_data[1]);
        mask = mask|(1<<irq);
        outb(pic_data[1], mask);
    }
}

void pic_acknowledge( uint8_t irq )
{
    if(irq>=8) {
        outb(pic_control[1], PIC_ACK_SPECIFIC+(irq-8));
        outb(pic_control[0], PIC_ACK_SPECIFIC+(2));
    } else {
        outb(pic_control[0], PIC_ACK_SPECIFIC+irq);
    }
}

/* Each of the IRQ ISRs point to this function, rather than
*  the 'fault_handler' in 'isrs.c'. The IRQ Controllers need
*  to be told when you are done servicing them, so you need
*  to send them an "End of Interrupt" command (0x20). There
*  are two 8259 chips: The first exists at 0x20, the second
*  exists at 0xA0. If the second controller (an IRQ from 8 to
*  15) gets an interrupt, you need to acknowledge the
*  interrupt at BOTH controllers, otherwise, you only send
*  an EOI command to the first controller. If you don't send
*  an EOI, you won't raise any more IRQs */
void irq_handler(struct regs *r)
{
    /* This is a blank function pointer */
    void (*handler)(struct regs *r);

    /* Find out if we have a custom handler to run for this
    *  IRQ, and then finally, run it */
    handler = irq_routines[r->int_no - 32];
    if (handler)
    {
        handler(r);
    }

    /* If the IDT entry that was invoked was greater than 40
    *  (meaning IRQ8 - 15), then we need to send an EOI to
    *  the slave controller */
    if (r->int_no >= 40)
    {
        outb(0xA0, 0x20);
    }

    /* In either case, we need to send an EOI to the master
    *  interrupt controller too */
    outb(0x20, 0x20);
}
