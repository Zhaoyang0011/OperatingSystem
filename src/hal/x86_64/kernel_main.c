#include <config_x86_64.h>
#include <hal/halglobal.h>
#include <hal/halinit.h>
#include <ldrtype.h>
#include <memory.h>
#include <spinlock.h>

void copy_kernel_desc()
{
    kernel_desc_t *temp = (kernel_desc_t *)(KERNEL_START + KERNEL_DESC_OFF);
    memcpy(temp, &kernel_info, sizeof(kernel_desc_t));

    if (kernel_info.kernel_magic != ZHOS_MAGIC)
    {
        while (TRUE)
            ;
    }
}

void kernel_main()
{
    copy_kernel_desc();

    init_hal();

    //
    while (TRUE)
        ;
}