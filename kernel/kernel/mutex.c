// hey look a basekernel file!
// https://github.com/dthain/basekernel/blob/6fff9df12906787b16ba94d685c3ec5bf28eb1eb/src/mutex.c

#include <kernel/mutex.h>
#include <kernel/interrupts.h>

void mutex_lock(struct mutex *m) {
	disable_interrupts();
	while(m->locked) {
		process_wait(&m->waitqueue);
		disable_interrupts();
	}
	m->locked=1;
	enable_interrupts();
}

void mutex_unlock(struct mutex *m) {
	disable_interrupts();
	m->locked=0;
	process_wakeup(&m->waitqueue);
	enable_interrupts();
}
