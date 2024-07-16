#ifndef _CONTEXT_H
#define _CONTEXT_H

#include <type.h>
#include <hal/cpu.h>

#ifdef x86_64

typedef struct context {
  uint_t ctx_nextrip;    //保存下一次运行的地址
  uint_t ctx_nextrsp;    //保存下一次运行时内核栈的地址
  x64tss_t *ctx_nexttss; //指向tss结构
} context_t;

typedef struct kernel_stack_registers {
  uint_t r_gs;
  uint_t r_fs;
  uint_t r_es;
  uint_t r_ds;
  uint_t r_r15;
  uint_t r_r14;
  uint_t r_r13;
  uint_t r_r12;
  uint_t r_r11;
  uint_t r_r10;
  uint_t r_r9;
  uint_t r_r8;
  uint_t r_rdi;
  uint_t r_rsi;
  uint_t r_rbp;
  uint_t r_rdx;
  uint_t r_rcx;
  uint_t r_rbx;
  uint_t r_rax;
  uint_t r_rip_old;
  uint_t r_cs_old;
  uint_t r_rflgs;
  uint_t r_rsp_old;
  uint_t r_ss_old;
} krlstkregs_t;

typedef struct thread thread_t;

void context_init(context_t *);
void switch_context(thread_t *next, thread_t *prev);
void retnfrom_first_sched(thread_t *thrdp);

void idlethread_init();
void idlethread_start();

#endif

#endif
