#include "memgrob.h"
#include <hal/memory/memdivmer.h>
#include <spinlock.h>

void update_memarea(memarea_t *mareap, uint_t retpnr, uint_t flgs)
{
}

void update_memmgrob(uint_t retpnr, uint_t flgs)
{
}

memarea_t *memarea_by_type(memgrob_t *mmobjp, uint_t mrtype)
{
    for (uint_t mi = 0; mi < mmobjp->mo_mareanr; mi++)
    {
        if (mrtype == mmobjp->mo_mareastat[mi].ma_type)
        {
            return &mmobjp->mo_mareastat[mi];
        }
    }
    return NULL;
}

mpdesc_t *mem_maxdivpages_onmarea(memarea_t *mareap, uint_t *retpnr)
{
    return NULL;
}

bool_t scan_mpgsalloc_ok(memarea_t *mareap, uint_t pages)
{
    if (mareap == NULL || pages == 0)
    {
        return FALSE;
    }
    if (mareap->ma_freepages >= pages && mareap->ma_maxpages >= pages)
    {
        return TRUE;
    }
    return FALSE;
}

bool_t onmpgs_retn_mpafhlst(memarea_t *mareap, uint_t pages, mpaflist_t **retrelbhl, mpaflist_t **retdivbhl)
{
    return FALSE;
}

mpdesc_t *mm_reldpgsdivmsa_bafhl(memarea_t *mareap, uint_t pages, uint_t *retpnr, mpaflist_t *retrelbhl,
                                 mpaflist_t *retdivbhl)
{
    return NULL;
}

mpdesc_t *mem_divpages_onmarea(memarea_t *mareap, uint_t pages, uint_t *retpnr)
{
    if (mareap == NULL || pages == 0 || retpnr == NULL)
    {
        *retpnr = 0;
        return NULL;
    }

    if (scan_mpgsalloc_ok(mareap, pages) == FALSE)
    {
        *retpnr = 0;
        return NULL;
    }

    mpaflist_t *retrelbhl = NULL, *retdivbhl = NULL;
    bool_t rets = onmpgs_retn_mpafhlst(mareap, pages, &retrelbhl, &retdivbhl);
    if (FALSE == rets)
    {
        *retpnr = 0;
        return NULL;
    }
    mpdesc_t *retmpg = mm_reldpgsdivmsa_bafhl(mareap, pages, retpnr, retrelbhl, retdivbhl);
    if (retmpg == NULL)
    {
        *retpnr = 0;
        return NULL;
    }
    return retmpg;
}

mpdesc_t *memory_divide_pages_core(memarea_t *mareap, uint_t pages, uint_t *retrelpnr, uint_t flgs)
{
    if (flgs != DMF_RELDIV && flgs != DMF_MAXDIV)
    {
        *retrelpnr = 0;
        return NULL;
    }
    if (mareap->ma_type != MA_TYPE_KRNL && mareap->ma_type != MA_TYPE_HWAD)
    {
        *retrelpnr = 0;
        return NULL;
    }

    uint_t retpnr = 0;
    mpdesc_t *retmpg = NULL;
    cpuflg_t cpuflg;

    spinlock_cli(&mareap->ma_lock, &cpuflg);

    if (DMF_MAXDIV == flgs)
    {
        retmpg = mem_maxdivpages_onmarea(mareap, &retpnr);
    }
    else if (DMF_RELDIV == flgs)
    {
        retmpg = mem_divpages_onmarea(mareap, pages, &retpnr);
    }
    if (retmpg != NULL && retpnr != 0)
    {
        update_memarea(mareap, retpnr, 0);
        update_memmgrob(retpnr, 0);
    }

    spinunlock_sti(&mareap->ma_lock, &cpuflg);

    *retrelpnr = retpnr;
    return retmpg;
}

// 内存分配页面接口
// mmobjp->内存管理数据结构指针 memory management structure pointer
// pages->请求分配的内存页面数 page number needed
// retrealpnr->存放实际分配内存页面数的指针 return real page number
// matype->请求的分配内存页面的内存区类型 memory area type
// flgs->请求分配的内存页面的标志位 flags
mpdesc_t *mm_divpages_framework(memgrob_t *mmobjp, uint_t pages, uint_t *retrelpnr, uint_t matype, uint_t flgs)
{

    memarea_t *marea = memarea_by_type(mmobjp, matype);
    if (marea == NULL)
    {
        *retrelpnr = 0;
        return NULL;
    }
    uint_t retpnr = 0;
    return memory_divide_pages_core(marea, pages, retrelpnr, flgs);
}

mpdesc_t *memory_divide_pages(memgrob_t *mmobjp, uint_t pages, uint_t *retrealpnr, uint_t mrtype, uint_t flgs)
{
    if (mmobjp == NULL || retrealpnr == NULL || mrtype == 0)
        return NULL;

    return mm_divpages_framework(mmobjp, pages, retrealpnr, mrtype, flgs);
}