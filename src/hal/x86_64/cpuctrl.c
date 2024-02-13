#include <spinlock.h>

void init_spinlock(spinlock_t *spinlock)
{
    spinlock->lock = 0;
}