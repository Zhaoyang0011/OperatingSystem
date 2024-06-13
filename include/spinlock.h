#ifndef _SPIN_LOCK_H
#define _SPIN_LOCK_H

#include <type.h>

typedef struct spinlock {
    volatile uint32_t lock;
} spinlock_t;

void spinlock_init(spinlock_t *spinlock);

void spin_lock(spinlock_t *lock);

void spin_unlock(spinlock_t *lock);

void spinlock_cli(spinlock_t *lock, cpuflg_t *cpuflg);

void spinunlock_sti(spinlock_t *lock, cpuflg_t *cpuflg);

#endif