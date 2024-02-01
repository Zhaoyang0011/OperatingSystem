#ifndef _SPIN_LOCK_H
#define _SPIN_LOCK_H

#include <type.h>

typedef struct spinlock
{
    volatile uint32_t lock;
} spinlock_t;

void init_spinlock(spinlock_t* spinlock)
{
    spinlock->lock = 0;
}

#endif