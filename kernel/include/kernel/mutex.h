#ifndef _KERNEL_MUTEX_H
#define _KERNEL_MUTEX_H

#include "list.h"

struct mutex {
	int locked;
	struct list waitqueue;
};

#define MUTEX_INIT {0,LIST_INIT}

void mutex_lock(struct mutex *m);
void mutex_unlock(struct mutex *m);

#endif
