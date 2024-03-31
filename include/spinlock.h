#ifndef _SPIN_LOCK_H
#define _SPIN_LOCK_H

#include <type.h>

typedef struct spinlock
{
    volatile uint32_t lock;
} spinlock_t;

void spinlock_init(spinlock_t *spinlock);

KLINE void spin_lock(spinlock_t *spinlock);

KLINE void spin_unlock(spinlock_t *spinlock);

#endif