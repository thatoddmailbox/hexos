# In just a few pages in this tutorial, we will add our Interrupt
# Service Routines (ISRs) right here!
.globl isr0
.globl isr1
.globl isr2
.globl isr3
.globl isr4
.globl isr5
.globl isr6
.globl isr7
.globl isr8
.globl isr9
.globl isr10
.globl isr11
.globl isr12
.globl isr13
.globl isr14
.globl isr15
.globl isr16
.globl isr17
.globl isr18
.globl isr19
.globl isr20
.globl isr21
.globl isr22
.globl isr23
.globl isr24
.globl isr25
.globl isr26
.globl isr27
.globl isr28
.globl isr29
.globl isr30
.globl isr31

.globl irq0
.globl irq1
.globl irq2
.globl irq3
.globl irq4
.globl irq5
.globl irq6
.globl irq7
.globl irq8
.globl irq9
.globl irq10
.globl irq11
.globl irq12
.globl irq13
.globl irq14
.globl irq15

.macro isr_noerror number
    cli
    pushl $0
    pushl \number
    jmp isr_common_stub
.endm

.macro isr_error number
    cli
    pushl \number
    jmp isr_common_stub
.endm

.macro irq_noerror number
    cli
    pushl $0
    pushl \number
    jmp irq_common_stub
.endm

isr0:
    isr_noerror $0

isr1:
    isr_noerror $1

isr2:
    isr_noerror $2

isr3:
    isr_noerror $3

isr4:
    isr_noerror $4

isr5:
    isr_noerror $5

isr6:
    isr_noerror $6

isr7:
    isr_noerror $7

isr8:
    isr_error $8

isr9:
    isr_noerror $9

isr10:
    isr_error $10

isr11:
    isr_error $11

isr12:
    isr_error $12

isr13:
    isr_error $13

isr14:
    isr_error $14

isr15:
    isr_noerror $15

isr16:
    isr_noerror $16

isr17:
    isr_noerror $17

isr18:
    isr_noerror $18

# RESERVED
isr19:
    isr_noerror $19

isr20:
    isr_noerror $20

isr21:
    isr_noerror $21

isr22:
    isr_noerror $22

isr23:
    isr_noerror $23

isr24:
    isr_noerror $24

isr25:
    isr_noerror $25

isr26:
    isr_noerror $26

isr27:
    isr_noerror $27

isr28:
    isr_noerror $28

isr29:
    isr_noerror $29

isr30:
    isr_noerror $30

isr31:
    isr_noerror $31

# IRQs
irq0:
    irq_noerror $32

irq1:
    irq_noerror $33

irq2:
    irq_noerror $34

irq3:
    irq_noerror $35

irq4:
    irq_noerror $36

irq5:
    irq_noerror $37

irq6:
    irq_noerror $38

irq7:
    irq_noerror $39

irq8:
    irq_noerror $40

irq9:
    irq_noerror $41

irq10:
    irq_noerror $42

irq11:
    irq_noerror $43

irq12:
    irq_noerror $44

irq13:
    irq_noerror $45

irq14:
    irq_noerror $46

irq15:
    irq_noerror $47

# We call a C function in here. We need to let the assembler know
# that 'fault_handler' exists in another file
.extern fault_handler

.intel_syntax noprefix
# This is our common ISR stub. It saves the processor state, sets
# up for kernel mode segments, calls the C-level fault handler,
# and finally restores the stack frame.
isr_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10   # Load the Kernel Data Segment descriptor!
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp   # Push us the stack
    push eax
    #mov eax, fault_handler
    #call eax       # A special call, preserves the 'eip' register
    call fault_handler
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8     # Cleans up the pushed error code and pushed ISR number
    iret           # pops 5 things at once: CS, EIP, EFLAGS, SS, and ESP!

.att_syntax

.extern irq_handler

.intel_syntax noprefix
# This is a stub that we have created for IRQ based ISRs. This calls
# '_irq_handler' in our C code. We need to create this in an 'irq.c'
irq_common_stub:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax
    #mov eax, irq_handler
    #call eax
    call irq_handler
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret
.att_syntax
