#ifndef _X86_H
#define _X86_H

#include <ldrtype.h>
#include <type.h>

#define CPUID_FLAG_MSR (1 << 5)

KLINE uint32_t cpuid(cpuid_desc_t *cpuid_desc) {
  __asm__ __volatile__("cpuid"
      : "=a"(*cpuid_desc->info), "=b"(*(cpuid_desc->info + 1)), "=c"(*(cpuid_desc->info + 2)), "=d"(*(cpuid_desc->info + 3))
      : "a"(cpuid_desc->function_id));
  return (uint32_t)cpuid_desc->info[0];
}

KLINE void outb(uint16_t port, uint8_t data) {
  __asm__ __volatile__("out %0,%1" : : "a"(data), "dN"(port));
}

KLINE uchar_t inb(uint16_t port) {
  uchar_t data;
  __asm__ __volatile__("in %1,%0" : "=a"(data) : "dN"(port));
  return data;
}

KLINE void insl(int port, void *addr, int cnt) {
  asm volatile("cld; rep insl" :
      "=D" (addr), "=c" (cnt) :
      "d" (port), "0" (addr), "1" (cnt) :
      "memory", "cc");
}

KLINE uint_t read_cr2() {
  uint_t regtmp = 0;
  __asm__ __volatile__(
      "movq %%cr2,%0\n\t"
      : "=r"(regtmp)
      :
      : "memory");
  return regtmp;
}

#endif