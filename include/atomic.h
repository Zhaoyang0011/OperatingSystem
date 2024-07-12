#ifndef _ATOMIC_H
#define _ATOMIC_H

#include <type.h>

#ifdef x86_64

typedef struct atomic {
  volatile uint32_t a_count;
} atomic_t;

KLINE uint32_t atomic_get(atomic_t *v) {
  return (*(volatile uint32_t *)&(v)->a_count);
}

KLINE void atomic_set(atomic_t *v, uint32_t value) {
  v->a_count = value;
}

KLINE void atomic_inc(atomic_t *v) {
  __asm__ __volatile__("lock;"
                       "incl %0" : "+m"(v->a_count));
}

KLINE void atomic_dec(atomic_t *v) {
  __asm__ __volatile__("lock;"
                       "decl %0" : "+m"(v->a_count));
}

KLINE void atomic_add(atomic_t *v, uint32_t i) {
  __asm__ __volatile__("lock;"
                       "addl %0, %1" : "+m"(v->a_count) : "ir"(i));
}

KLINE void atomic_sub(atomic_t *v, uint32_t i) {
  __asm__ __volatile__("lock;"
                       "subl %0, %1" : "+m"(v->a_count) : "ir"(i));
}

typedef atomic_t refcount_t;

#define refcount_inc(v) atomic_inc(v)

#define refcount_dec(v) atomic_dec(v)

#endif

#endif