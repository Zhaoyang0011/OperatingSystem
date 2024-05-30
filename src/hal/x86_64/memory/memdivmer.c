#include "../io.h"
#include <hal/memory/memdivmer.h>
#include <hal/memory/memgrob.h>
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

KLINE sint_t retn_divoder(uint_t pages)
{
    sint_t pbits = search_64rlbits((uint_t)pages) - 1;
    if (pages & (pages - 1))
    {
        pbits++;
    }
    return pbits;
}

// get mpaflist on page number
bool_t get_mpaflist_onpages(memarea_t *mareap, uint_t pages, mpaflist_t **prelbhl, mpaflist_t **pdivbhl)
{
    if (mareap == NULL || pages == 0 || prelbhl == NULL || pdivbhl == NULL)
        return FALSE;

    // 获取bafhlst_t结构数组的开始地址
    mpaflist_t *pmpaflst = mareap->ma_mdmdata.dm_mdmlst;
    // 根据分配页面数计算出分配页面在dm_mdmlielst数组中下标
    sint_t dividx = retn_divoder(pages);
    if (dividx < 0 || dividx >= MDIVMER_ARR_LMAX || pages > pmpaflst[dividx].af_oderpnr)
    {
        *prelbhl = NULL;
        *pdivbhl = NULL;
        return FALSE;
    }
    // 从第dividx个数组元素开始搜索
    for (sint_t idx = dividx; idx < MDIVMER_ARR_LMAX; ++idx)
    {
        // 如果第idx个数组元素对应的一次可分配连续的页面数大于等于请求的页面数，且其中的可分配对象大于0则返回
        if (pmpaflst[idx].af_oderpnr >= pages && pmpaflst[idx].af_fobjnr > 0)
        {
            // 返回请求分配的bafhlst_t结构指针
            *prelbhl = &pmpaflst[dividx];
            // 返回实际分配的bafhlst_t结构指针
            *pdivbhl = &pmpaflst[idx];
            return TRUE;
        }
    }
    *prelbhl = NULL;
    *pdivbhl = NULL;
    return FALSE;
}

bool_t ret_mpg_onmpaflist_core(mpaflist_t *bafhp, mpdesc_t **retmstat, mpdesc_t **retmend)
{
    if (NULL == bafhp || NULL == retmstat || NULL == retmend)
    {
        return FALSE;
    }
    if (1 > bafhp->af_mobjnr || 1 > bafhp->af_fobjnr)
    {
        *retmstat = NULL;
        *retmend = NULL;
        return FALSE;
    }
    if (list_is_empty_careful(&bafhp->af_frelist) == TRUE)
    {
        *retmstat = NULL;
        *retmend = NULL;
        return FALSE;
    }
    mpdesc_t *tmp = list_entry(bafhp->af_frelist.next, mpdesc_t, mpd_list);
    list_del(&tmp->mpd_list);
    bafhp->af_mobjnr--;
    bafhp->af_fobjnr--;
    bafhp->af_freindx++;
    *retmstat = tmp;
    *retmend = (mpdesc_t *)tmp->mpd_odlink;
    if (MF_OLKTY_BAFH == tmp->mpd_indxflgs.mpf_olkty)
    {
        *retmend = tmp;
    }
    return TRUE;
}

// get memory pages on struct mpaflist
mpdesc_t *get_mpg_onmpaflist(uint_t pages, uint_t *retpnr, mpaflist_t *relbhl, mpaflist_t *divbhl)
{
    if (pages == 0 || retpnr == NULL || relbhl == NULL || divbhl == NULL)
        return NULL;

    if (relbhl > divbhl)
    {
        *retpnr = 0;
        return NULL;
    }
    mpdesc_t *retmsa = NULL;
    bool_t rets = FALSE;
    mpdesc_t *retmstat = NULL, *retmend = NULL;
    if (relbhl == divbhl)
    {
        bool_t res = ret_mpg_onmpaflist_core(relbhl, &retmstat, &retmend);
        if (res == FALSE)
        {
            *retpnr = 0;
            return NULL;
        }
        *retpnr = retmend - retmstat;
        return retmstat;
    }

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
    bool_t rets = get_mpaflist_onpages(mareap, pages, &retrelbhl, &retdivbhl);
    if (FALSE == rets)
    {
        *retpnr = 0;
        return NULL;
    }

    mpdesc_t *retmpg = get_mpg_onmpaflist(pages, retpnr, retrelbhl, retdivbhl);
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