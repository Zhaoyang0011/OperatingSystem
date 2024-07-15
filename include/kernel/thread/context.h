#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include <type.h>

typedef struct context context_t;
typedef struct kernel_stack_registers krlstkregs_t;

#ifdef x86_64

typedef struct x64tss {
  uint32_t reserv0;    //保留
  uint64_t rsp0;       //R0特权级的栈地址
  uint64_t rsp1;       //R1特权级的栈地址，我们未使用
  uint64_t rsp2;       //R2特权级的栈地址，我们未使用
  uint64_t reserv1;    //保留
  uint64_t ist[7];     //我们未使用
  uint64_t reserv2;    //保留
  uint64_t reserv3;    //保留
  uint64_t iobase;     //我们未使用
}__attribute__((packed)) x64tss_t;

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

void context_init(context_t *);

#endif

#endif
