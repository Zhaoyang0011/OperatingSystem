#include "ldr.h"
#include <type.h>

#define VIDEO_MEM 0xB8000

void setup_die()
{
    char *video_mem = (char *)VIDEO_MEM;
    video_mem[0] = 'D';
    video_mem[1] = 15;
    video_mem[2] = 'I';
    video_mem[3] = 15;
    video_mem[4] = 'E';
    video_mem[5] = 15;
    while (TRUE)
        ;
}

/**
 * This is the main function of the setup.
 * It will call other functions that prepares our device information.
 */
void setup_main()
{
    kernel_desc_t *kernel_desc = (kernel_desc_t *)(KERNEL_START + KERNEL_DESC_OFF);

    chkini_kernel_info(kernel_desc);
    init_stack(kernel_desc);
    init_memory_info(kernel_desc);
    init_pages(kernel_desc);
}