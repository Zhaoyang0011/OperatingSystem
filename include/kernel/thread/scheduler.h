#ifndef _SCHEDULER_H
#define _SCHEDULER_H

#include <struct/list.h>
#include <kernel/thread/thread.h>
#include <hal/halglobal.h>
#include "hal/cpu.h"

#define NOTS_SCHED_FLGS (0)
#define NEED_SCHED_FLGS (1)
#define NEED_START_CPUILDE_SCHED_FLGS (2)
#define PMPT_FLGS 0

typedef struct thread_list {
  list_t tdl_lsth;
  thread_t *tdl_curruntd;
  uint_t tdl_nr;
} thrdlst_t;

typedef struct scheduler_data {
  spinlock_t sda_lock;
  uint_t sda_cpuid;
  uint_t sda_schdflgs;
  uint_t sda_premptidx;
  uint_t sda_threadnr;
  uint_t sda_prityidx;
  thread_t *sda_cpuidle;
  thread_t *sda_currtd;
  thrdlst_t sda_thdlst[PRITY_MAX];
} schdata_t;

typedef struct scheduler_class {
  spinlock_t scls_lock;
  uint_t scls_cpunr;
  uint_t scls_threadnr;
  uint_t scls_threadid_inc;
  schdata_t scls_schda[CPUCORE_MAX];
} schedclass_t;

void thrdlst_init(thrdlst_t *initp);

void schdata_init(schdata_t *initp);

void schedclass_t_init(schedclass_t *initp);

void init_krlsched();

// global scheduler for os
extern KRL_DEFGLOB_VARIABLE(schedclass_t, scheduler);

thread_t *scheduler_retn_curthread();
thread_t *scheduler_select_thread();
void scheduler_add_thread(thread_t *td);

#endif