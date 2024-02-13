#include <kprint.h>
#include <hal/cpuctrl.h>
#include <hal/halglobal.h>
#include <hal/phymem.h>

void init_one_phymem(e820_map_t *e820, physical_memory_t *pm)
{
    init_spinlock(&pm->lock);
    pm->pm_saddr = e820->addr;
    pm->pm_size = e820->size;
    pm->pm_end = e820->addr + e820->size;
}

void init_halphymem_core(e820_map_t *e820, physical_memory_t *pm, uint64_t num)
{
    for (int i = 0; i < num; ++i)
    {
        init_one_phymem(e820 + i, pm + i);
    }
}

void init_halphymem()
{
    if (kernel_info.mmap_adr == 0 && kernel_info.mmap_nr == 0)
    {
        kprint("");
    }
    physical_memory_t *pm_sadr = (physical_memory_t *)kernel_info.next_pg;
    e820_map_t *e820arr = (e820_map_t *)kernel_info.mmap_adr;

    init_halphymem_core(e820arr, pm_sadr, kernel_info.mmap_nr);

    kernel_info.mmap_adr = (uint64_t)pm_sadr;
    kernel_info.mmap_sz = (uint64_t)(kernel_info.mmap_nr * sizeof(physical_memory_t));
    kernel_info.next_pg = P4K_ALIGN(kernel_info.mmap_adr + kernel_info.mmap_sz);
}