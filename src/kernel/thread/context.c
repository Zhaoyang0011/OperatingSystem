#include <kernel/thread/context.h>
#include <kernel/thread/scheduler.h>
#include <hal/cpu.h>

HAL_DEFGLOB_VARIABLE(x64tss_t, x64_tss)[CPU_CORE_MAX];

//初始化context_t结构
void context_init(context_t *initp) {
  initp->ctx_nextrip = 0;
  initp->ctx_nextrsp = 0;
  //指向当前CPU的tss
  initp->ctx_nexttss = &x64_tss[cur_cpuid()];
}