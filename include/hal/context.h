#ifndef _CONTEXT_H_
#define _CONTEXT_H_

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

#endif

#endif
