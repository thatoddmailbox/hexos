#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

// Defines a GDT entry
struct gdt_entry
{
    unsigned short limit_low;
    unsigned short base_low;
    unsigned char base_middle;
    unsigned char access;
    unsigned char granularity;
    unsigned char base_high;
} __attribute__((packed)); // packed so gcc doesn't mess with it

// Special pointer which includes the limit: The max bytes taken up by the GDT, minus 1.
struct gdt_ptr
{
    unsigned short limit;
    unsigned int base;
} __attribute__((packed)); // packed so gcc doesn't mess with it

/* Our GDT, with 3 entries, and finally our special GDT pointer */
struct gdt_entry gdt[3];
struct gdt_ptr gp;

// This is a function in boot.s. It is used to properly reload the new segment registers
extern void gdt_flush();

void gdt_set_gate(int num, unsigned long base, unsigned long limit, unsigned char access, unsigned char gran);
void gdt_install();

#endif
