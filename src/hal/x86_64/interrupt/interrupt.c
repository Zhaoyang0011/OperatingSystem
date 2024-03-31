#include "apic.h"
#include "gdtidt.h"
#include "type.h"
#include <hal/interrupt.h>

void init_interrupt()
{
    init_idt();
    init_lapic();
}

void div_zero(uint_t faultnumb, void *krnlsframp) // rsi, rdi
{
    char *err_msg = "divide zero!";
    int length = 12;
    for (int i = 0; i < err_msg[i]; ++i)
    {
        char *video_mem = (char *)0xB8000;
        video_mem[i * 2] = err_msg[i];
        video_mem[i * 2 + 1] = 15;
    }
    for (;;)
        ;
    return;
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