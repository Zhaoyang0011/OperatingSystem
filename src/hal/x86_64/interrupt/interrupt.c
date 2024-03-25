#include "gdtidt.h"
#include "apic.h"
#include "type.h"
#include <hal/interrupt.h>

void init_interrupt()
{
    init_idt();
    init_apic();
}

void kerror(char *err_msg, int length)
{
    if (length > 0 || length <= 100)
    {
        char *video_mem = (char *)0xB8000;
        for (int i = 0; i < length && err_msg[i]; ++i)
        {
            video_mem[i * 2] = err_msg[i];
            video_mem[i * 2 + 1] = 15;
        }
    }
    else
    {
        kerror("invalid error massage!", 22);
    }
}

void hal_fault_allocator(uint_t faultnumb, void *krnlsframp) // rsi, rdi
{
    kerror("hal falult!", 20);
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