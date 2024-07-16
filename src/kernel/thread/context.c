#include <kernel/thread/context.h>
#include <kernel/thread/scheduler.h>
#include <hal/cpu.h>

#ifdef x86_64

HAL_DEFGLOB_VARIABLE(x64tss_t, x64_tss)[CPU_CORE_MAX];

//初始化context_t结构
void context_init(context_t *initp) {
  initp->ctx_nextrip = 0;
  initp->ctx_nextrsp = 0;
  //指向当前CPU的tss
  initp->ctx_nexttss = &x64_tss[cur_cpuid()];
}

void __to_new_context(thread_t *next, thread_t *prev) {
  uint_t cpuid = cur_cpuid();
  schdata_t *schdap = &scheduler.scls_schda[cpuid];

  schdap->sda_currtd = next;

  next->td_context.ctx_nexttss = &x64_tss[cpuid];
  next->td_context.ctx_nexttss->rsp0 = next->td_krlstktop;
  hal_mmu_load(&next->td_mmdsc->msd_mmu);
  if (next->td_stus == TDSTUS_NEW) {
    next->td_stus = TDSTUS_RUN;
    retnfrom_first_sched(next);
  }
}

void retnfrom_first_sched(thread_t *thrdp) {
  __asm__ __volatile__(
      "movq %[NEXT_RSP],%%rsp\n\t"
      "popq %%r14\n\t"
      "movw %%r14w,%%gs\n\t"
      "popq %%r14\n\t"
      "movw %%r14w,%%fs\n\t"
      "popq %%r14\n\t"
      "movw %%r14w,%%es\n\t"
      "popq %%r14\n\t"
      "movw %%r14w,%%ds\n\t"
      "popq %%r15\n\t"
      "popq %%r14\n\t"
      "popq %%r13\n\t"
      "popq %%r12\n\t"
      "popq %%r11\n\t"
      "popq %%r10\n\t"
      "popq %%r9\n\t"
      "popq %%r8\n\t"
      "popq %%rdi\n\t"
      "popq %%rsi\n\t"
      "popq %%rbp\n\t"
      "popq %%rdx\n\t"
      "popq %%rcx\n\t"
      "popq %%rbx\n\t"
      "popq %%rax\n\t"
      "iretq\n\t"

      :
      : [ NEXT_RSP ] "m"(thrdp->td_context.ctx_nextrsp)
  : "memory");
}

void switch_context(thread_t *next, thread_t *prev) {
  __asm__ __volatile__(
      "pushfq \n\t"//保存当前进程的标志寄存器
      "cli \n\t"  //关中断
      //保存当前进程的通用寄存器
      "pushq %%rax\n\t"
      "pushq %%rbx\n\t"
      "pushq %%rcx\n\t"
      "pushq %%rdx\n\t"
      "pushq %%rbp\n\t"
      "pushq %%rsi\n\t"
      "pushq %%rdi\n\t"
      "pushq %%r8\n\t"
      "pushq %%r9\n\t"
      "pushq %%r10\n\t"
      "pushq %%r11\n\t"
      "pushq %%r12\n\t"
      "pushq %%r13\n\t"
      "pushq %%r14\n\t"
      "pushq %%r15\n\t"
      //保存CPU的RSP寄存器到当前进程的机器上下文结构中
      "movq %%rsp,%[PREV_RSP] \n\t"
      //把下一个进程的机器上下文结构中的RSP的值，写入CPU的RSP寄存器中
      "movq %[NEXT_RSP],%%rsp \n\t"//事实上这里已经切换到下一个进程了，因为切换进程的内核栈
      //调用__to_new_context函数切换MMU页表
      "callq __to_new_context\n\t"
      //恢复下一个进程的通用寄存器
      "popq %%r15\n\t"
      "popq %%r14\n\t"
      "popq %%r13\n\t"
      "popq %%r12\n\t"
      "popq %%r11\n\t"
      "popq %%r10\n\t"
      "popq %%r9\n\t"
      "popq %%r8\n\t"
      "popq %%rdi\n\t"
      "popq %%rsi\n\t"
      "popq %%rbp\n\t"
      "popq %%rdx\n\t"
      "popq %%rcx\n\t"
      "popq %%rbx\n\t"
      "popq %%rax\n\t"
      "popfq \n\t"      //恢复下一个进程的标志寄存器
    //输出当前进程的内核栈地址
      : [ PREV_RSP ] "=m"(prev->td_context.ctx_nextrsp)
  //读取下一个进程的内核栈地址
  : [ NEXT_RSP ] "m"(next->td_context.ctx_nextrsp), "D"(next), "S"(prev)//为调用__to_new_context函数传递参数
  : "memory");
}

void idlethread_init() {
  uint_t cpuid = cur_cpuid();
  thread_t *ret_td = new_cpuidle_thread();
  scheduler.scls_schda[cpuid].sda_cpuidle = ret_td;
  scheduler.scls_schda[cpuid].sda_currtd = ret_td;
}

void idlethread_start() {
  uint_t cpuid = cur_cpuid();
  schdata_t *schdap = &scheduler.scls_schda[cpuid];

  thread_t *tdp = schdap->sda_cpuidle;
  tdp->td_context.ctx_nexttss = &x64_tss[cpuid];
  tdp->td_context.ctx_nexttss->rsp0 = tdp->td_krlstktop;
  tdp->td_stus = TDSTUS_RUN;
  retnfrom_first_sched(tdp);
}

#endif