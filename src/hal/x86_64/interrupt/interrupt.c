#include "gdtidt.h"
#include "type.h"
#include <hal/interrupt.h>

void init_interrupt()
{
    init_idt();
}

void hal_fault_allocator(uint_t faultnumb, void *krnlsframp) // rax,rdx
{
    for (;;)
        ;
    return;
}

sint_t hal_syscl_allocator(uint_t sys_nr, void *msgp)
{
    return 0;
}

void hal_hwint_allocator(uint_t intnumb, void *krnlsframp) //rax,rdx
{
    for (;;)
        ;
    return;
}