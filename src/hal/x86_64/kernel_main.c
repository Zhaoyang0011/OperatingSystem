#include <config_x86_64.h>
#include <hal/halglobal.h>
#include <hal/halinit.h>
#include <hal/interrupt.h>
#include <kprint.h>
#include <ldrtype.h>
#include <spinlock.h>
#include <string.h>
#include <type.h>

void copy_kernel_desc()
{
    kernel_desc_t *temp = (kernel_desc_t *)(KERNEL_START + KERNEL_DESC_OFF);
    memcpy(temp, &kernel_descriptor, sizeof(kernel_desc_t));

    if (kernel_descriptor.kernel_magic != ZHOS_MAGIC)
    {
        kprint("Incorrect kernel head!");
        while (TRUE)
            ;
    }

    if (kernel_descriptor.kernel_size == 0)
    {
        kprint("");
        while (TRUE)
            ;
    }

    kernel_descriptor.next_pg = P4K_ALIGN(kernel_descriptor.next_pg);
    kernel_descriptor.kernel_size = kernel_descriptor.next_pg - KERNEL_START;
}

void kernel_main()
{
    
    copy_kernel_desc();
    init_hal();

    //
    while (TRUE)
        ;
}