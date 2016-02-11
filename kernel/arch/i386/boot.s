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
stack_top:

# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
	movl $stack_top, %esp

	# multiboot info
	pushl   %ebx # pointer to the info struct
	pushl   %eax # magic value

	# Initialize the core kernel before running the global constructors.
	call kernel_early

	# Call the global constructors.
	call _init

	# Transfer control to the main kernel.
	call kernel_main

	# Hang if kernel_main unexpectedly returns.
	cli
.Lhang:
	hlt
	jmp .Lhang

# This will set up our new segment registers. We need to do
# something special in order to set CS. We do what is called a
# far jump. A jump that includes a segment as well as an offset.
# This is declared in C as 'extern void gdt_flush();'
#.globl gdt_flush     # Allows the C code to link to this
#.extern gp            # Says that '_gp' is in another file
#gdt_flush:
#    lgdt gp        # Load the GDT with our '_gp' which is a special pointer
#    mov $0x10, %ax      # 0x10 is the offset in the GDT to our data segment
#    mov %ax, %ds
#    mov %ax, %es
#    mov %ax, %fs
#    mov %ax, %gs
#    mov %ax, %ss
#    pushw $0x08   # 0x08 is the offset to our code segment: Far jump!
#	push flush2
#	retf
#flush2:
#    ret               # Returns back to the C code!

# Loads the IDT defined in '_idtp' into the processor.
# This is declared in C as 'extern void idt_load();'
.extern idtp
.globl idt_load
idt_load:
    lidt idtp
    ret


.size _start, . - _start
