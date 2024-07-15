#ifndef _IO_H
#define _IO_H

#include <type.h>

KLINE sint_t search_64rlbits(uint64_t val) {
  sint_t retbitnr = -1;
  __asm__ __volatile__("bsrq %1,%q0 \t\n" : "+r"(retbitnr) : "rm"(val));
  return retbitnr + 1;
}

KLINE sint_t search_64lbits(uint64_t val) {
  sint_t retbitnr = -1;
  __asm__ __volatile__("bsrq %1,%q0 \t\n" : "+r"(retbitnr) : "rm"(val));
  return retbitnr;
}

KLINE void write_cr3(uint_t r_val) {
  __asm__ __volatile__(

      "movq %0,%%cr3 \n\t"
      :
      : "r"(r_val)
      : "memory" //, "edx"
      );
}

#endif