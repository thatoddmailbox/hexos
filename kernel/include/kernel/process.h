#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#include <kernel/list.h>

#define PROCESS_STATE_CRADLE  0
#define PROCESS_STATE_READY   1
#define PROCESS_STATE_RUNNING 2
#define PROCESS_STATE_BLOCKED 3
#define PROCESS_STATE_GRAVE   4

#define X86_SEGMENT_SELECTOR( seg, rpl )  (((seg)<<3)+(rpl))

#define X86_SEGMENT_KERNEL_CODE  X86_SEGMENT_SELECTOR(1,0)
#define X86_SEGMENT_KERNEL_DATA  X86_SEGMENT_SELECTOR(2,0)
#define X86_SEGMENT_USER_CODE    X86_SEGMENT_SELECTOR(3,3)
#define X86_SEGMENT_USER_DATA    X86_SEGMENT_SELECTOR(4,3)
#define X86_SEGMENT_TSS          X86_SEGMENT_SELECTOR(5,0)

struct x86_eflags {
	unsigned carry:1;
	unsigned reserved0:1;
	unsigned parity:1;
	unsigned reserved1:1;

	unsigned auxcarry:1;
	unsigned reserved2:1;
	unsigned zero:1;
	unsigned sign:1;

	unsigned trap:1;
	unsigned interrupt:1;
	unsigned direction:1;
	unsigned overflow:1;

	unsigned iopl:2;
	unsigned nested:1;
	unsigned reserved3:1;

	unsigned resume:1;
	unsigned v86:1;
	unsigned align:1;
	unsigned vinterrupt:1;

	unsigned vpending:1;
	unsigned id:1;
};

struct x86_regs {
	int32_t	eax;
	int32_t	ebx;
	int32_t	ecx;
	int32_t	edx;
	int32_t	esi;
	int32_t	edi;
	int32_t	ebp;
};

struct x86_stack {
	struct x86_regs		regs2;
	int32_t			old_ebp;
	int32_t			old_addr;
	struct x86_regs		regs1;
	int32_t			ds;
	int32_t			intr_num;
	int32_t			intr_code;
	int32_t			eip;
	int32_t			cs;
	struct x86_eflags	eflags;
	int32_t			esp;
	int32_t			ss;
};

struct process {
	struct list_node node;
	int state;
	int exitcode;
	char *name;
	struct pagetable *pagetable;
	char *kstack;
	char *kstack_top;
	char *stack_ptr;
	uint32_t entry;
};

void process_init();

struct process * process_create( unsigned code_size, unsigned stack_size );
void process_yield();
void process_preempt();
void process_exit(int code, char * extra_info);
void process_dump( struct process *p );
void process_dump_current();

static void process_switch(int newstate);

void process_print_all(struct list *q);
void process_print_all_ready();

void process_wait( struct list *q );
void process_wakeup( struct list *q );
void process_wakeup_all( struct list *q );

extern struct process *current;

#endif
