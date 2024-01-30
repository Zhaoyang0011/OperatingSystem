#include <ldrtype.h>

#include "ldr.h"

#define VIDEO_MEM 0xB8000

void die()
{
    while (TRUE)
        ;
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
    }
    else
    {
        kerror("invalid error massage!", 22);
    }
    die();
}

void init_stack()
{
    kernel_desc_t *kernel_desc = (kernel_desc_t *)(KERNEL_START + KERNEL_DESC_OFF);
    kernel_desc->stack_init_adr = STACK_PHYADR;
    kernel_desc->stack_size = STACK_SIZE;
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
    uint64_t *p = (uint64_t *)KINITPAGE_PHYADR;
    uint64_t *pdpte = (uint64_t *)(KINITPAGE_PHYADR + 0x1000);
    uint64_t *pde = (uint64_t *)(KINITPAGE_PHYADR + 0x2000);

    for (int i = 0; i < PGENTY_SIZE; i++)
    {
        p[i] = 0;
        pdpte[i] = 0;
    }

    // 
    p[0] = (uint64_t)((uint32_t)pdpte | KPML4_RW | KPML4_P);
    p[(KRNL_VIRTUAL_ADDRESS_START >> 39) & 0x1ff] = (uint64_t)((uint32_t)pdpte | KPML4_RW | KPML4_P);

    uint64_t adr = 0;
    for (uint_t i = 0; i < 16; i++)
    {
        pdpte[i] = (uint64_t)((uint32_t)pde | KPDPTE_RW | KPDPTE_P);
        for (uint_t j = 0; j < PGENTY_SIZE; j++)
        {
            pde[j] = 0 | adr | KPDE_PS | KPDE_RW | KPDE_P;
            adr += (0x1 << KPDP_SHIFT);
        }
        pde = (uint64_t *)((uint64_t)pde + 0x1000);
    }
}

void init_machine_param()
{
    kernel_desc_t *kernel_desc = (kernel_desc_t *)(KERNEL_START + KERNEL_DESC_OFF);
    if (kernel_desc->kernel_magic != ZHOS_MAGIC)
    {
        char *error_msg = "Invalid kernel magic number";
        kerror(error_msg, 28);
    }

    init_stack();
    init_memory_info();
    init_pages();
}