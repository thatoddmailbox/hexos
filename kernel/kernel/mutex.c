// hey look a basekernel file!
// https://github.com/dthain/basekernel/blob/6fff9df12906787b16ba94d685c3ec5bf28eb1eb/src/mutex.c

void mutex_lock(struct mutex *m) {
	interrupt_block();
	while(m->locked) {
		process_wait(&m->waitqueue);
		interrupt_block();
	}
	m->locked=1;
	interrupt_unblock();
}

void mutex_unlock(struct mutex *m) {
	interrupt_block();
	m->locked=0;
	process_wakeup(&m->waitqueue);
	interrupt_unblock();
}
