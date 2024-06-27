#ifndef _CPU_H
#define _CPU_H

#include <kernel/thread.h>

#define CPU_CORE_MAX 16

struct cpu {
  thread_t *cpu_curthread;
} cpu_t;

#endif