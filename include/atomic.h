#ifndef _ATOMIC_H
#define _ATOMIC_H

#include <type.h>

typedef struct s_ATOMIC {
  volatile sint32_t a_count;
} atomic_t;

typedef struct s_REFCOUNT {
  atomic_t ref_count;
} refcount_t;

#endif