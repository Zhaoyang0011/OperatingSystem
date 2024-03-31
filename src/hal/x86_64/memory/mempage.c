#include "hal/memory/phymem.h"
#include <console.h>
#include <hal/halglobal.h>
#include <hal/memory/mempage.h>
#include <ldrtype.h>

bool_t ret_msadsc_vadrandsz(kernel_desc_t *kernel, mpdesc_t **retmpsadr, uint64_t *retmpnr)
{
    if (kernel == NULL || retmpsadr == NULL || retmpnr == NULL)
        return FALSE;

    if (kernel->mmap_nr < 1 || NULL == kernel->mmap_adr ||
        (kernel->mmap_nr * sizeof(physical_memory_t)) != kernel->mmap_sz)
    {
        *retmpsadr = NULL;
        *retmpnr = 0;
        return FALSE;
    }

    uint64_t memory_size = 0, memory_page_num = 0;
    physical_memory_t *pmarr = (physical_memory_t *)kernel->mmap_adr;
    for (uint64_t i = 0; i < kernel->mmap_nr; i++)
    {
        if (pmarr[i].pm_type == PM_T_OSAPUSERRAM)
        {
            memory_size += pmarr[i].pm_size;
            memory_page_num += (pmarr[i].pm_size >> 12);
        }
    }

    if (memory_size == 0 || (memory_size >> 1) < 1 || memory_page_num < 1)
    {
        *retmpsadr = NULL;
        *retmpnr = 0;
        return FALSE;
    }

    *retmpsadr = (mpdesc_t *)kernel->next_pg;
    *retmpnr = memory_page_num;

    return TRUE;
}

void init_mpdsc(mpdesc_t *mp)
{
    list_init(&mp->mpd_list);
    spinlock_init(&mp->mpd_lock);
    mp->mpd_indxflgs.mpf_olkty = MF_OLKTY_INIT;
    mp->mpd_indxflgs.mpf_lstty = MF_LSTTY_LIST;
    mp->mpd_indxflgs.mpf_mocty = MF_MOCTY_FREE;
    mp->mpd_indxflgs.mpf_marty = MF_MARTY_INIT;
    mp->mpd_indxflgs.mpf_uindx = MF_UINDX_INIT;

    mp->mpd_phyadrs.paf_alloc = PAF_NO_ALLOC;
    mp->mpd_phyadrs.paf_shared = PAF_NO_SHARED;
    mp->mpd_phyadrs.paf_swap = PAF_NO_SWAP;
    mp->mpd_phyadrs.paf_cache = PAF_NO_CACHE;
    mp->mpd_phyadrs.paf_kmap = PAF_NO_KMAP;
    mp->mpd_phyadrs.paf_lock = PAF_NO_LOCK;
    mp->mpd_phyadrs.paf_dirty = PAF_NO_DIRTY;
    mp->mpd_phyadrs.paf_busy = PAF_NO_BUSY;
    mp->mpd_phyadrs.paf_rsv = PAF_RSV_VAL;
    mp->mpd_phyadrs.paf_padrs = PAF_INIT_PADRS;
    mp->mpd_odlink = NULL;
}

void write_one_mpdsc(mpdesc_t *mp, uint64_t phyadr)
{
    init_mpdsc(mp);
	phyadrflgs_t *tmp = (phyadrflgs_t *)(&phyadr);
	mp->mpd_phyadrs.paf_padrs = tmp->paf_padrs;
}

uint64_t init_mpdsc_core(kernel_desc_t *kernel, mpdesc_t *mpsadr, uint64_t mpnr)
{
    if (kernel == NULL || mpsadr == NULL || mpnr < 1)
        return 0;

    physical_memory_t *pmarr = (physical_memory_t *)kernel->mmap_adr;

    uint64_t retmpnr = 0;
    for (uint64_t i = 0; i < kernel->mmap_nr; i++)
    {
        if (PM_T_OSAPUSERRAM != pmarr[i].pm_type)
            continue;

        for (uint64_t start = pmarr[i].pm_start; start < pmarr[i].pm_end; start += 4096)
        {
            if ((start + 4096 - 1) <= pmarr[i].pm_end)
            {
                write_one_mpdsc(&mpsadr[retmpnr], start);
                retmpnr++;
            }
        }
    }

    return retmpnr;
}

void init_memory_page()
{
    uint64_t corempnr = 0, mpnr = 0;
    mpdesc_t *mpsadr = NULL;
    kernel_desc_t *kernel = &kernel_descriptor;
    if (ret_msadsc_vadrandsz(kernel, &mpsadr, &mpnr) == FALSE)
    {
        panic("init_msadsc ret_msadsc_vadrandsz err\n");
    }

    corempnr = init_mpdsc_core(kernel, mpsadr, mpnr);
    if (corempnr != mpnr)
    {
        panic("init_msadsc_core err\n");
    }

    kernel_descriptor.mp_desc_arr = (uint64_t)mpsadr;
    kernel_descriptor.mp_desc_nr = corempnr;
    kernel_descriptor.mp_desc_sz = corempnr * sizeof(mpdesc_t);

    kernel_descriptor.next_pg = P4K_ALIGN(kernel_descriptor.mp_desc_arr + kernel_descriptor.mp_desc_sz);
}