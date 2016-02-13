# Declare constants used for creating a multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Reserve a stack for the initial thread.
.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 # 16 KiB
INTERRUPT_STACK_TOP:

#include "kernel/memorylayout.h"

# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
	movl $INTERRUPT_STACK_TOP, %esp

	pushl	%esp # stack start location
	# multiboot info
	pushl   %ebx # pointer to the info struct
	pushl   %eax # magic value

	mov	$5*8, %ax

	# Initialize the core kernel before running the global constructors.
	call kernel_early

	# Call the global constructors.
	#call _init

	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
.Lhang:
	hlt
	jmp .Lhang

# The TSS is a big task management structure used by the 386.
# We do not use the TSS, but simply rely on pushing variables
# around in stacks.  However, we need to use the TSS in order
# to initialize the stack pointer and segment for priv level 0

.align 16
.global tss
tss:
	.long	0

.global interrupt_stack_pointer
interrupt_stack_pointer:
	.long	INTERRUPT_STACK_TOP # initial interrupt stack ptr at 64 KB
	.long	2*8		    # use segment 2 for the interrupt stack
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0
	.long	0

.global intr_return
intr_return:
	popl	%eax
syscall_return:
	popl	%ebx
	popl	%ecx
	popl	%edx
	popl	%esi
	popl	%edi
	popl	%ebp
	popl	%ds
	addl	$4, %esp	# remove interrupt num
	addl	$4, %esp	# remove detail code
	iret			# iret gets the intr context

.global gdt_flush
.extern gp
.intel_syntax noprefix
# "but wait! Comments in Intel syntax are with semicolons"
# "hahahahahaha" - gnu assembler

# This will set up our new segment registers. We need to do
# something special in order to set CS. We do what is called a
# far jump. A jump that includes a segment as well as an offset.
# This is declared in C as 'extern void gdt_flush();'
gdt_flush:
    lgdt [gp]        # Load the GDT with our '_gp' which is a special pointer
    mov ax, 0x10      # 0x10 is the offset in the GDT to our data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:flush2   # 0x08 is the offset to our code segment: Far jump!
flush2:
    ret               # Returns back to the C code!

.att_syntax
# Loads the IDT defined in '_idtp' into the processor.
# This is declared in C as 'extern void idt_load();'
.extern idtp
.globl idt_load
.intel_syntax noprefix
idt_load:
    lidt [idtp]
    ret

.size _start, . - _start
