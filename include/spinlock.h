#ifndef _SPIN_LOCK_H
#define _SPIN_LOCK_H

#include <type.h>

typedef struct spinlock
{
    volatile uint32_t lock;
} spinlock_t;

#endif