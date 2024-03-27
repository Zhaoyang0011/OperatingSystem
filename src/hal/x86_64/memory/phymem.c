#include <hal/halglobal.h>
#include <hal/memory/phymem.h>
#include <kprint.h>
#include <spinlock.h>
#include <type.h>

int init_one_phymem(physical_memory_t *pm)
{
    init_spinlock(&pm->lock);
    pm->pm_type = 0;
    pm->pm_stype = 0;
    pm->pm_flags = 0;
    pm->pm_status = 0;
    pm->pm_start = 0;
    pm->pm_size = 0;
    pm->pm_end = 0;
    pm->pm_rsv_start = 0;
    pm->pm_rsv_end = 0;
}

int set_one_phymem(e820_map_t *e820, physical_memory_t *pm)
{
    init_one_phymem(pm);

    uint32_t ptype = 0, pstype = 0;
    switch (e820->type)
    {
    case RAM_USABLE:
        ptype = PM_T_OSAPUSERRAM;
        pstype = RAM_USABLE;
        break;
    case RAM_RESERV:
        ptype = PM_T_RESERVRAM;
        pstype = RAM_RESERV;
        break;
    case RAM_ACPIREC:
        ptype = PM_T_HWUSERRAM;
        pstype = RAM_ACPIREC;
        break;
    case RAM_ACPINVS:
        ptype = PM_T_HWUSERRAM;
        pstype = RAM_ACPINVS;
        break;
    case RAM_AREACON:
        ptype = PM_T_BUGRAM;
        pstype = RAM_AREACON;
        break;
    default:
        break;
    }

    if (0 == ptype)
        return FALSE;

    pm->pm_type = ptype;
    pm->pm_stype = pstype;
    pm->pm_flags = PM_F_X86_64;
    pm->pm_start = e820->addr;
    pm->pm_size = e820->size;
    pm->pm_end = e820->addr + e820->size - 1;

    return TRUE;
}

void sort_phymem(physical_memory_t *pm, uint64_t num)
{
}

void init_phymem_core(e820_map_t *e820, physical_memory_t *pm, uint64_t num)
{
    for (int i = 0; i < num; ++i)
    {
        set_one_phymem(e820 + i, pm + i);
    }

    sort_phymem(pm, num);
}

void init_physical_memory()
{
    if (kernel_descriptor.mmap_adr == 0 || kernel_descriptor.mmap_nr == 0)
    {
        kprint("Memory map incorrect!");
        while (TRUE)
            ;
    }

    physical_memory_t *pm_sadr = (physical_memory_t *)kernel_descriptor.next_pg;
    e820_map_t *e820arr = (e820_map_t *)kernel_descriptor.mmap_adr;

    init_phymem_core(e820arr, pm_sadr, kernel_descriptor.mmap_nr);

    kernel_descriptor.mmap_adr = (uint64_t)pm_sadr;
    kernel_descriptor.mmap_sz = (uint64_t)(kernel_descriptor.mmap_nr * sizeof(physical_memory_t));
    kernel_descriptor.next_pg = P4K_ALIGN(kernel_descriptor.mmap_adr + kernel_descriptor.mmap_sz);
    kernel_descriptor.kernel_size = kernel_descriptor.next_pg - kernel_descriptor.kernel_start;
}