#include "apic.h"
#include "gdtidt.h"
#include "type.h"
#include <hal/interrupt.h>

void init_interrupt()
{
    init_idt();
    init_lapic();
}

void hal_fault_allocator(uint_t faultnumb, void *krnlsframp) // rsi, rdi
{
    for (;;)
        ;
    return;
}

sint_t hal_syscl_allocator(uint_t sys_nr, void *msgp)
{
    return 0;
}

void hal_hwint_allocator(uint_t intnumb, void *krnlsframp) // rsi, rdi
{
    for (;;)
        ;
    return;
}