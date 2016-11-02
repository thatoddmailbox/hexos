/*
 * The vast majority of this code is from/based off of basekernel.
 * You can see the specific file here: https://github.com/dthain/basekernel/blob/6fff9df12906787b16ba94d685c3ec5bf28eb1eb/src/memory.c
 */

#include <kernel/interrupts.h>
#include <kernel/mem.h>
#include <kernel/process.h>
#include <kernel/memorylayout.h>

struct process *current=0;
struct list ready_list = {0,0};
bool hasInited = false;

void process_init() {
	//process_create(0,0);
	printf("process_init 1\n");
	current = process_create(0,0);
	printf("process_init 2\n");

	current->name = "kernel";

	printf("process_init 3\n");
	pagetable_load(current->pagetable);
	printf("process_init 4\n");
	pagetable_enable();
	printf("process_init 5\n");

	current->state = PROCESS_STATE_READY;
	//list_push_tail(&ready_list,&current->node);

	dbgprint("process: ready\n");
	hasInited = true;
}

void process_test() {
	//process_create(0,0);
	//process_switch(PROCESS_STATE_READY);
	struct process *test = process_create(128,0);

	test->name = "test";

	pagetable_load(test->pagetable);
	pagetable_enable();

	*((char*)0x80000000) = 0xcd;
	*((char*)0x80000001) = 0x80;

	test->state = PROCESS_STATE_READY;

	list_push_tail(&ready_list,&test->node);
}

static void process_stack_init(struct process *p) {
	struct x86_stack *s;

	p->state = PROCESS_STATE_CRADLE;

	p->kstack_top = p->kstack+PAGE_SIZE-sizeof(*s);
	p->stack_ptr = p->kstack_top;

	s = (struct x86_stack *) p->stack_ptr;

	s->regs2.ebp = (uint32_t) (p->stack_ptr + 28);
	s->old_ebp = (uint32_t) (p->stack_ptr + 32);
	s->old_addr = (unsigned) intr_return;
	s->ds = X86_SEGMENT_USER_DATA;
	s->cs = X86_SEGMENT_USER_CODE;
	s->eip = p->entry;
	s->eflags.interrupt = 1;
	s->eflags.iopl = 3;
	s->esp = PROCESS_STACK_INIT;
	s->ss = X86_SEGMENT_USER_DATA;
}

struct process * process_create(unsigned code_size, unsigned stack_size) {
	struct process *p;

	p = memory_alloc_page(1);

	p->pagetable = pagetable_create();
	pagetable_init(p->pagetable);
	pagetable_alloc(p->pagetable,PROCESS_ENTRY_POINT,code_size,PAGE_FLAG_USER|PAGE_FLAG_READWRITE);
	pagetable_alloc(p->pagetable,PROCESS_STACK_INIT-stack_size,stack_size,PAGE_FLAG_USER|PAGE_FLAG_READWRITE);

	p->kstack = memory_alloc_page(1);
	p->entry = PROCESS_ENTRY_POINT;

	process_stack_init(p);

	return p;
}

static void process_switch(int newstate) {
	disable_interrupts();

	if(current) {
		if(current->state!=PROCESS_STATE_CRADLE) {
			asm("pushl %ebp");
			asm("pushl %edi");
			asm("pushl %esi");
			asm("pushl %edx");
			asm("pushl %ecx");
			asm("pushl %ebx");
			asm("pushl %eax");
			asm("movl %%esp, %0" : "=r" (current->stack_ptr));
		}
		interrupt_stack_pointer = (void*)INTERRUPT_STACK_TOP;
		current->state = newstate;
		if(newstate==PROCESS_STATE_READY) {
			list_push_tail(&ready_list,&current->node);
		}
	}

	current = 0;

	while(1) {
		current = (struct process *) list_pop_head(&ready_list);
		if(current) {
			break;
		}
		enable_interrupts();
		wait_interrupts();
		disable_interrupts();
	}

	current->state = PROCESS_STATE_RUNNING;
	interrupt_stack_pointer = current->kstack_top;
	asm("movl %0, %%cr3" :: "r" (current->pagetable));
	asm("movl %0, %%esp" :: "r" (current->stack_ptr));

	asm("popl %eax");
	asm("popl %ebx");
	asm("popl %ecx");
	asm("popl %edx");
	asm("popl %esi");
	asm("popl %edi");
	asm("popl %ebp");

	enable_interrupts();
}

int allow_preempt=0;

void process_preempt() {
	if(allow_preempt && current && ready_list.head) {
		process_switch(PROCESS_STATE_READY);
	}
}

void process_yield() {
	process_switch(PROCESS_STATE_READY);
}

void process_exit(int code, char * extra_info) {
	printf("process exited with status %d\n",code);
	if (extra_info != 0) {
		printf("%s\n", extra_info);
	}
	current->exitcode = code;
	process_switch(PROCESS_STATE_GRAVE);
}

void process_wait(struct list * q) {
	list_push_tail(q,&current->node);
	process_switch(PROCESS_STATE_BLOCKED);
}

void process_wakeup(struct list *q) {
	struct process *p;
	p = (struct process *)list_pop_head(q);
	if(p) {
		p->state = PROCESS_STATE_READY;
		list_push_tail(&ready_list,&p->node);
	}
}

void process_wakeup_all(struct list *q) {
	struct process *p;
	while((p = (struct process*)list_pop_head(q))) {
		p->state = PROCESS_STATE_READY;
		list_push_tail(&ready_list,&p->node);
	}
}

void process_print_all(struct list *q) {
	struct process *p;
	while((p = (struct process*)list_pop_head(q))) {
		printf("%s (state: %d)\n", p->name, p->state);
	}
}

void process_print_all_ready() {
	process_print_all(&ready_list);
}

void process_dump( struct process *p) {
	struct x86_stack *s = (struct x86_stack*)(p->kstack+PAGE_SIZE-sizeof(*s));
	printf("kstack: %x\n",p->kstack);
	printf("stackp: %x\n",p->stack_ptr);
	printf("eax: %x     cs: %x\n",s->regs1.eax,s->cs);
	printf("ebx: %x     ds: %x\n",s->regs1.ebx,s->ds);
	printf("ecx: %x     ss: %x\n",s->regs1.ecx,s->ss);
	printf("edx: %x eflags: %x\n",s->regs1.edx,s->eflags);
	printf("esi: %x\n",s->regs1.esi);
	printf("edi: %x\n",s->regs1.edi);
	printf("ebp: %x\n",s->regs1.ebp);
	printf("esp: %x\n",s->esp);
	printf("eip: %x\n",s->eip);
}

void process_dump_current() {
	process_dump(current);
}
