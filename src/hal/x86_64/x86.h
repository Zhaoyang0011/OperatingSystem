#ifndef _X86_H
#define _X86_H

#include <ldrtype.h>
#include <type.h>

#define CPUID_FLAG_MSR (1 << 5)

KLINE uint32_t cpuid(cpuid_desc_t *cpuid_desc)
{
    __asm__ __volatile__("cpuid"
                         : "=a"(*cpuid_desc->info), "=b"(*(cpuid_desc->info + 1)), "=c"(*(cpuid_desc->info + 2)),
                           "=d"(*(cpuid_desc->info + 3))
                         : "a"(cpuid_desc->function_id));
    return (uint32_t)cpuid_desc->info[0];
}

#endif