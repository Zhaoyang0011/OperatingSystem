#include <kernel/krlinit.h>
#include <kernel/virmem.h>
#include <kernel/krlmempool.h>
#include <kernel/thread/scheduler.h>
#include "kprint.h"
void thread_a_main() {
  uint_t i = 0;
  for (;; i++) {
    kprint("process a running:%x\n", i);
    schedule();
  }
}

void thread_b_main() {
  uint_t i = 0;
  for (;; i++) {
    kprint("process b running:%x\n", i);
    schedule();
  }
}

void init_ab_thread() {
  krl_new_thread((void *)thread_a_main, KERNTHREAD_FLG,
                 PRILG_SYS, PRITY_MIN, DAFT_TDUSRSTKSZ, DAFT_TDKRLSTKSZ);
  krl_new_thread((void *)thread_b_main, KERNTHREAD_FLG,
                 PRILG_SYS, PRITY_MIN, DAFT_TDUSRSTKSZ, DAFT_TDKRLSTKSZ);
}

void init_krl() {
  pvmspacdesc_init(&initial_pvmspacedesc);
  kmempool_init(&oskmempool);
  schedclass_init(&scheduler);

  idlethread_init();
  init_ab_thread();
  idlethread_start();
}