#ifndef _TYPE_H
#define _TYPE_H

#define KLINE static inline

#define NULL (0) // null pointer

typedef unsigned char uint8_t;
typedef unsigned char uchar_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef unsigned int size_t;
typedef unsigned long uint_t;

typedef char sint8_t;
typedef char schar_t;
typedef short sint16_t;
typedef int sint32_t;
typedef long long sint64_t;
typedef long sint_t;
typedef unsigned long long addr_t;

typedef sint_t bool_t;
typedef uint64_t cpuflg_t;

#define ALIGN(x, a) (((x) + (a)-1) & ~((a)-1))
#define P4K_ALIGN(x) ALIGN(x, 0x1000)

#define TRUE 1
#define FALSE 0

#define EXTERN extern
#define KEXTERN extern

#endif