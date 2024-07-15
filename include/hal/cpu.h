#ifndef _CPU_H
#define _CPU_H

#define CPU_CORE_MAX 16
#include <type.h>

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

#endif

int cur_cpuid();

#endif