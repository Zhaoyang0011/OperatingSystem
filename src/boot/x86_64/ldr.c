#include <ldrtype.h>

#include "ldr.h"

#define VIDEO_MEM 0xB8000

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
    while (TRUE)
        ;
}

// check and init kernel info
void chkini_kernel_info(kernel_desc_t *kernel_desc)
{
    kernel_desc->kernel_start = KERNEL_START;
    if (kernel_desc->kernel_magic != ZHOS_MAGIC)
    {
        char *error_msg = "Invalid kernel magic number";
        kerror(error_msg, 28);
    }
}

void init_stack(kernel_desc_t *kernel_desc)
{
    kernel_desc->stack_init_adr = STACK_PHYADR;
    kernel_desc->stack_size = STACK_SIZE;
}

e820_map_t *chk_memsize(e820_desc_t *e8p, uint64_t addr, uint64_t size)
{
    uint64_t len = addr + size;
    if (e8p == NULL || e8p->e820_num == 0)
    {
        return NULL;
    }
    for (uint32_t i = 0; i < e8p->e820_num; i++)
    {
        if ((e8p->maps[i].type == RAM_USABLE) && (addr >= e8p->maps[i].addr) && (len < (e8p->maps[i].addr + e8p->maps[i].size)))
        {
            return &e8p->maps[i];
        }
    }
    return NULL;
}

uint64_t get_memsize(e820_desc_t *e820_desc)
{
    uint64_t size = 0;
    for (int i = 0; i < e820_desc->e820_num; ++i)
    {
        if (e820_desc->maps[i].type == RAM_USABLE)
        {
            size += e820_desc->maps[i].size;
        }
    }
    return size;
}

//
void init_memory_info(kernel_desc_t *kernel_desc)
{
    e820_desc_t *e820_desc = (e820_desc_t *)E820_DESC;
    if (e820_desc->e820_num == 0)
    {
        kerror("No e820map", 20);
    }
    if (chk_memsize(e820_desc, 0x100000, 0x8000000) == 0)
    {
        kerror("Your computer is low on memory, the memory cannot be less than 128MB!", 20);
    }

    kernel_desc->mmap_adr = (uint64_t)(e820_desc->maps);
    kernel_desc->mmap_nr = (uint64_t)e820_desc->e820_num;
    kernel_desc->mmap_sz = (uint64_t)(e820_desc->e820_num * sizeof(e820_map_t));
    kernel_desc->mach_memsize = get_memsize(e820_desc);
}

/**
 * This funcion prepares the memory page information that the long mode needed.
 * To simplify programming difficulty, we use 2M as the page size.
 * After entering long mode we will modify the paging settings and use 4K as the page size.
 */
void init_pages(kernel_desc_t *kernel_desc)
{
    uint64_t *p = (uint64_t *)KINITPAGE_PHYADR;
    uint64_t *pdpte = (uint64_t *)(KINITPAGE_PHYADR + 0x1000);
    uint64_t *pde = (uint64_t *)(KINITPAGE_PHYADR + 0x2000);

    for (int i = 0; i < PGENTY_SIZE; i++)
    {
        p[i] = 0;
        pdpte[i] = 0;
    }

    p[0] = (uint64_t)((uint32_t)pdpte | KPML4_RW | KPML4_P);
    p[(KRNL_VIRTUAL_ADDRESS_START >> 39) & 0x1ff] = (uint64_t)((uint32_t)pdpte | KPML4_RW | KPML4_P);

    kernel_desc->kernel_start = KERNEL_START;
    if (kernel_desc->kernel_magic != ZHOS_MAGIC)
    {
        char *error_msg = "Invalid kernel magic number";
        kerror(error_msg, 28);
    }

    uint64_t adr = 0;
    for (uint_t i = 0; i < 16; i++)
    {
        pdpte[i] = (uint64_t)((uint32_t)pde | KPDPTE_RW | KPDPTE_P);
        for (uint_t j = 0; j < PGENTY_SIZE; j++)
        {
            pde[j] = 0 | adr | KPDE_PS | KPDE_RW | KPDE_P;
            adr += (0x1 << KPDP_SHIFT);
        }
        pde = (uint64_t *)(((uint64_t)pde) + 0x1000);
    }
}

void init_machine_param()
{
    kernel_desc_t *kernel_desc = (kernel_desc_t *)(KERNEL_START + KERNEL_DESC_OFF);

    chkini_kernel_info(kernel_desc);
    init_stack(kernel_desc);
    init_memory_info(kernel_desc);
    init_pages(kernel_desc);
}