#include <hal/halglobal.h>
#include <hal/halinit.h>
#include <kernel/krlinit.h>
#include <ldrtype.h>
#include <string.h>
#include <type.h>
#include <console.h>

void copy_kernel_desc() {
    kernel_desc_t *temp = (kernel_desc_t *) (KERNEL_START + KERNEL_DESC_OFF);
    memcpy(temp, &kernel_descriptor, sizeof(kernel_desc_t));

    if (kernel_descriptor.kernel_magic != ZHOS_MAGIC) {
        panic("Incorrect kernel head!");
    }

    if (kernel_descriptor.kernel_size == 0) {
        panic("Kernel size incorrect!");
    }

    kernel_descriptor.next_pg = P4K_ALIGN(kernel_descriptor.next_pg);
    kernel_descriptor.kernel_size = kernel_descriptor.next_pg - KERNEL_START;
}

void kernel_main() {

    copy_kernel_desc();
    init_hal();
    init_krl();
    //
    while (TRUE);
}