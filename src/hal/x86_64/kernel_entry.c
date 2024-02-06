#include <config_x86_64.h>
#include <hal/halinit.h>
#include <hal/halglobal.h>
#include <ldrtype.h>
#include <memory.h>
#include <spinlock.h>

void copy_kernel_desc(kernel_desc_t *p_kernel_desc)
{
    kernel_desc_t *temp = (kernel_desc_t *)(KERNEL_START + KERNEL_DESC_OFF);
    memcpy(temp, p_kernel_desc, sizeof(kernel_desc_t));

    if (p_kernel_desc->kernel_magic != ZHOS_MAGIC)
    {
        while (TRUE)
            ;
    }
}

void kernel_main()
{
    copy_kernel_desc(&kernel_info);

    init_hal();
    
    //
    while (TRUE);
}