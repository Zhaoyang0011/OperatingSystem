#include "ldr.h"
#include <asm-generic/errno.h>

#define VIDEO_MEM 0xB8000

void die()
{
    while (TRUE);
}

void kerror(char *err_msg, int length)
{
    if (length > 0 || length <= 100)
    {
        for (int i = 0; i < length && err_msg[i]; ++i)
        {
            char *video_mem = (char *)VIDEO_MEM;
            video_mem[i * 2] = err_msg[i];
            video_mem[i * 2 + 1] = 15;
        }
    } else {
        kerror("invalid error massage!", 22);
    }
    die();
}

/**
 * This funcion initialize kernel information.
 */
void init_kernel_info()
{
    
}

void init_stack()
{
}

void init_memory_info()
{
}

/**
 * This funcion prepares the memory page information that the long mode needed.
 * To simplify programming difficulty, we use 2M as the page size.
 * After entering long mode we will modify the paging settings and use 4K as the page size.
 */
void init_pages()
{
}

void init_machine_param()
{
    init_kernel_info();
    init_stack();
    init_memory_info();
    init_pages();
}