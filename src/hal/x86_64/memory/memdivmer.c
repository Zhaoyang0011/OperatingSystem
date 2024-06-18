#include "../io.h"
#include "type.h"
#include <console.h>
#include <hal/memory/memdivmer.h>
#include <hal/memory/memgrob.h>
#include <spinlock.h>

void update_memarea(memarea_t *mareap, uint_t pgnr, uint_t flgs) {
    if (NULL == mareap) {
        return;
    }
    if (0 == flgs) {
        mareap->ma_freepages -= pgnr;
        mareap->ma_allocpages += pgnr;
    }
    if (1 == flgs) {
        mareap->ma_freepages += pgnr;
        mareap->ma_allocpages -= pgnr;
    }
}

void update_memmgrob(uint_t retpnr, uint_t flgs) {
    cpuflg_t cpuflg;
    if (0 == flgs) {
        spinlock_cli(&memgrob.mo_lock, &cpuflg);
        memgrob.mo_alocpages += retpnr;
        memgrob.mo_freepages -= retpnr;
        spinunlock_sti(&memgrob.mo_lock, &cpuflg);
    }
    if (1 == flgs) {
        spinlock_cli(&memgrob.mo_lock, &cpuflg);
        memgrob.mo_alocpages -= retpnr;
        memgrob.mo_freepages += retpnr;
        spinunlock_sti(&memgrob.mo_lock, &cpuflg);
    }
}

memarea_t *memarea_by_type(memgrob_t *mmobjp, uint_t mrtype) {
    for (uint_t mi = 0; mi < mmobjp->mo_mareanr; mi++) {
        if (mrtype == mmobjp->mo_mareastat[mi].ma_type) {
            return &mmobjp->mo_mareastat[mi];
        }
    }
    return NULL;
}

mpdesc_t *mem_maxdivpages_onmarea(memarea_t *mareap, uint_t *retpnr) {
    return NULL;
}

bool_t scan_mpgsalloc_ok(memarea_t *mareap, uint_t pages) {
    if (mareap == NULL || pages == 0) {
        return FALSE;
    }
    if (mareap->ma_freepages >= pages && mareap->ma_maxpages >= pages) {
        return TRUE;
    }
    return FALSE;
}

KLINE sint_t retn_divoder(uint_t pages) {
    sint_t pbits = search_64rlbits((uint_t) pages) - 1;
    if (pages & (pages - 1)) {
        pbits++;
    }
    return pbits;
}

// get mpaflist on page number
bool_t get_mpaflist_onpages(memarea_t *mareap, uint_t pages, mpaflist_t **prelbhl, mpaflist_t **pdivbhl) {
    if (mareap == NULL || pages == 0 || prelbhl == NULL || pdivbhl == NULL)
        return FALSE;

    // 获取bafhlst_t结构数组的开始地址
    mpaflist_t *pmpaflst = mareap->ma_mdmdata.dm_mdmlst;
    // 根据分配页面数计算出分配页面在dm_mdmlielst数组中下标
    sint_t dividx = retn_divoder(pages);
    if (dividx < 0 || dividx >= MDIVMER_ARR_LMAX || pages > pmpaflst[dividx].af_oderpnr) {
        *prelbhl = NULL;
        *pdivbhl = NULL;
        return FALSE;
    }
    // 从第dividx个数组元素开始搜索
    for (sint_t idx = dividx; idx < MDIVMER_ARR_LMAX; ++idx) {
        // 如果第idx个数组元素对应的一次可分配连续的页面数大于等于请求的页面数，且其中的可分配对象大于0则返回
        if (pmpaflst[idx].af_oderpnr >= pages && pmpaflst[idx].af_fobjnr > 0) {
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

bool_t ret_mpg_onmpaflist_core(mpaflist_t *bafhp, mpdesc_t **retmstat, mpdesc_t **retmend) {
    if (NULL == bafhp || NULL == retmstat || NULL == retmend) {
        return FALSE;
    }
    if (bafhp->af_mobjnr < 1 || bafhp->af_fobjnr < 1) {
        *retmstat = NULL;
        *retmend = NULL;
        return FALSE;
    }
    if (list_is_empty_careful(&bafhp->af_frelist) == TRUE) {
        *retmstat = NULL;
        *retmend = NULL;
        return FALSE;
    }
    mpdesc_t *tmp = list_entry(bafhp->af_frelist.next, mpdesc_t, mpd_list);
    list_del(&tmp->mpd_list);
    bafhp->af_mobjnr--;
    bafhp->af_fobjnr--;
    bafhp->af_alcindx++;
    *retmstat = tmp;
    *retmend = (mpdesc_t *) tmp->mpd_odlink;
    if (MF_OLKTY_BAFH == tmp->mpd_indxflgs.mpf_olkty) {
        *retmend = tmp;
    }
    return TRUE;
}

// set properties of divided mpdesc
void divpages_mpdesc(mpdesc_t *mpstart, uint_t mnr) {
    if (mpstart == NULL || mnr == 0) {
        return;
    }
    if ((mpstart->mpd_indxflgs.mpf_olkty != MF_OLKTY_ODER && MF_OLKTY_BAFH != mpstart->mpd_indxflgs.mpf_olkty) ||
        mpstart->mpd_odlink == NULL || mpstart->mpd_adrflgs.paf_alloc != PAF_NO_ALLOC) {
        panic("divpages_mpdesc err1\n");
    }

    mpdesc_t *mpend = (mpdesc_t *) mpstart->mpd_odlink;
    if (mpstart->mpd_indxflgs.mpf_olkty == MF_OLKTY_BAFH) {
        mpend = mpstart;
    }
    if (mpend < mpstart) {
        panic("divpages_mpdesc err2\n");
    }
    if ((uint_t) ((mpend - mpstart) + 1) != mnr) {
        panic("divpages_mpdesc err3\n");
    }
    if (mpstart->mpd_indxflgs.mpf_uindx > (MF_UINDX_MAX - 1) || mpend->mpd_indxflgs.mpf_uindx > (MF_UINDX_MAX - 1) ||
        mpstart->mpd_indxflgs.mpf_uindx != mpend->mpd_indxflgs.mpf_uindx) {
        panic("divpages_mpdesc err4");
    }
    if (mpend != mpstart) {
        mpend->mpd_indxflgs.mpf_uindx++;
        mpend->mpd_adrflgs.paf_alloc = PAF_ALLOC;
    }
    mpstart->mpd_indxflgs.mpf_uindx++;
    mpstart->mpd_adrflgs.paf_alloc = PAF_ALLOC;
    mpstart->mpd_indxflgs.mpf_olkty = MF_OLKTY_ODER;
    mpstart->mpd_odlink = mpend;
}

bool_t add_pages_mpaflist(mpaflist_t *aflst, mpdesc_t *mpstart, mpdesc_t *mpend) {
    uint_t mnr = (mpend - mpstart) + 1;
    if (mnr != (uint_t) aflst->af_oderpnr) {
        return FALSE;
    }
    if (0 < mpend->mpd_indxflgs.mpf_uindx || 0 < mpend->mpd_indxflgs.mpf_uindx) {
        return FALSE;
    }

    if (mpstart == mpend && 1 != mnr && 1 != aflst->af_oderpnr) {
        return FALSE;
    }
    mpstart->mpd_indxflgs.mpf_olkty = MF_OLKTY_ODER;
    mpstart->mpd_odlink = mpend;

    mpend->mpd_indxflgs.mpf_olkty = MF_OLKTY_BAFH;
    mpend->mpd_odlink = aflst;
    list_add(&mpstart->mpd_list, &aflst->af_frelist);
    aflst->af_mobjnr++;
    aflst->af_fobjnr++;
    return TRUE;
}

// get memory pages on struct mpaflist
mpdesc_t *get_mpg_onmpaflist(uint_t pages, uint_t *retpnr, mpaflist_t *relbhl, mpaflist_t *divbhl) {
    if (pages == 0 || retpnr == NULL || relbhl == NULL || divbhl == NULL)
        return NULL;

    if (relbhl > divbhl) {
        *retpnr = 0;
        return NULL;
    }
    mpdesc_t *retmsa = NULL;
    bool_t rets = FALSE;
    mpdesc_t *retmstat = NULL, *retmend = NULL;

    bool_t res = ret_mpg_onmpaflist_core(divbhl, &retmstat, &retmend);

    if (res == FALSE || retmend - retmstat != divbhl->af_oderpnr - 1) {
        *retpnr = 0;
        return NULL;
    }

    uint_t divnr = divbhl->af_oderpnr;

    if (relbhl == divbhl) {
        divpages_mpdesc(retmstat, divnr);
        *retpnr = retmend - retmstat + 1;
        return retmstat;
    }

    for (mpaflist_t *tmpbhl = divbhl - 1; tmpbhl >= relbhl; tmpbhl--) {
        if (add_pages_mpaflist(tmpbhl, &retmstat[tmpbhl->af_oderpnr], (mpdesc_t *) retmstat->mpd_odlink) == FALSE) {
            panic("mrdmb_add_msa_bafh fail\n");
        }
        retmstat->mpd_odlink = &retmstat[tmpbhl->af_oderpnr - 1];
        divnr -= tmpbhl->af_oderpnr;
    }

    divpages_mpdesc(retmstat, divnr);
    *retpnr = divnr;
    return retmstat;
}

mpdesc_t *mem_divpages_onmarea(memarea_t *mareap, uint_t pages, uint_t *retpnr) {
    if (mareap == NULL || pages == 0 || retpnr == NULL) {
        *retpnr = 0;
        return NULL;
    }

    if (scan_mpgsalloc_ok(mareap, pages) == FALSE) {
        *retpnr = 0;
        return NULL;
    }

    mpaflist_t *retrelbhl = NULL, *retdivbhl = NULL;
    bool_t rets = get_mpaflist_onpages(mareap, pages, &retrelbhl, &retdivbhl);
    if (rets == FALSE) {
        *retpnr = 0;
        return NULL;
    }

    mpdesc_t *retmpg = get_mpg_onmpaflist(pages, retpnr, retrelbhl, retdivbhl);
    if (retmpg == NULL) {
        *retpnr = 0;
        return NULL;
    }
    return retmpg;
}

// memory divide core function
mpdesc_t *memory_divide_pages_core(memarea_t *mareap, uint_t pages, uint_t *retrelpnr, uint_t flgs) {
    if (flgs != DMF_RELDIV && flgs != DMF_MAXDIV) {
        *retrelpnr = 0;
        return NULL;
    }
    if (mareap->ma_type != MA_TYPE_KRNL && mareap->ma_type != MA_TYPE_HWAD) {
        *retrelpnr = 0;
        return NULL;
    }

    uint_t retpnr = 0;
    mpdesc_t *retmpg = NULL;
    cpuflg_t cpuflg;

    spinlock_cli(&mareap->ma_lock, &cpuflg);

    do {
        if (DMF_MAXDIV == flgs) {
            retmpg = mem_maxdivpages_onmarea(mareap, &retpnr);
            break;
        }

        if (DMF_RELDIV == flgs) {
            retmpg = mem_divpages_onmarea(mareap, pages, &retpnr);
            break;
        }

        retpnr = 0;
        retmpg = NULL;
    } while (FALSE);

    if (retmpg != NULL && retpnr != 0) {
        update_memarea(mareap, retpnr, 0);
        update_memmgrob(retpnr, 0);
    }

    spinunlock_sti(&mareap->ma_lock, &cpuflg);

    *retrelpnr = retpnr;
    return retmpg;
}

mpdesc_t *mem_divpages_framework(memgrob_t *mmobjp, uint_t pages, uint_t *retrelpnr, uint_t matype, uint_t flgs) {

    memarea_t *marea = memarea_by_type(mmobjp, matype);
    if (marea == NULL) {
        *retrelpnr = 0;
        return NULL;
    }
    return memory_divide_pages_core(marea, pages, retrelpnr, flgs);
}

// 内存分配页面接口
// mmobjp->内存管理数据结构指针 memory management structure pointer
// pages->请求分配的内存页面数 page number needed
// retrealpnr->存放实际分配内存页面数的指针 return real divided page number
// matype->请求的分配内存页面的内存区类型 memory area type
// flgs->请求分配的内存页面的标志位 flags
mpdesc_t *memory_divide_pages(memgrob_t *mmobjp, uint_t pages, uint_t *retrealpnr, uint_t mrtype, uint_t flgs) {
    if (mmobjp == NULL || retrealpnr == NULL || mrtype == 0)
        return NULL;

    return mem_divpages_framework(mmobjp, pages, retrealpnr, mrtype, flgs);
}

/****************************************MERGE MEMORY PAGE***************************************************/

memarea_t *memarea_by_page(memgrob_t *mmobjp, mpdesc_t *fmstart, uint_t freepgs) {
    if (mmobjp == NULL || fmstart == NULL || freepgs < 1)
        return NULL;

    mpdesc_t *fmend = (mpdesc_t *) fmstart->mpd_odlink;
    if (fmend - fmstart + 1 != freepgs || fmstart->mpd_indxflgs.mpf_marty != fmend->mpd_indxflgs.mpf_marty)
        return NULL;

    for (uint_t mi = 0; mi < mmobjp->mo_mareanr; mi++) {
        if ((uint_t) (fmstart->mpd_indxflgs.mpf_marty) == mmobjp->mo_mareastat[mi].ma_type)
            return &mmobjp->mo_mareastat[mi];
    }

    return NULL;
}

bool_t scan_freemsa_isok(mpdesc_t *freemsa, uint_t freepgs) {
    if (freemsa == NULL || freepgs < 1)
        return FALSE;

    if (MF_OLKTY_ODER != freemsa->mpd_indxflgs.mpf_olkty || NULL == freemsa->mpd_odlink ||
        1 > freemsa->mpd_indxflgs.mpf_uindx)
        return FALSE;

    mpdesc_t *end = (mpdesc_t *) freemsa->mpd_odlink;

    if (PAF_ALLOC != freemsa->mpd_adrflgs.paf_alloc || PAF_ALLOC != end->mpd_adrflgs.paf_alloc ||
        end->mpd_indxflgs.mpf_uindx < 1)
        return FALSE;

    if (((uint_t) ((end - freemsa) + 1)) != freepgs)
        return FALSE;

    return TRUE;
}

bool_t onpgs_ret_mpafhlst(memarea_t *marea, uint_t freepgs, mpaflist_t **retrelbf, mpaflist_t **retmerbf) {
    if (marea == 0 || freepgs < 1 || retrelbf == NULL || retmerbf == NULL)
        return FALSE;
    mpaflist_t *bafhstat = marea->ma_mdmdata.dm_mdmlst;
    sint_t dividx = retn_divoder(freepgs);
    if (0 > dividx || MDIVMER_ARR_LMAX <= dividx) {
        *retrelbf = NULL;
        *retmerbf = NULL;
        return FALSE;
    }
    if ((~0UL) <= bafhstat[dividx].af_mobjnr) {
        panic("onfpgs_retn_bafhlst af_mobjnr max");
    }
    if ((~0UL) <= bafhstat[dividx].af_fobjnr) {
        panic("onfpgs_retn_bafhlst af_fobjnr max");
    }

    if (freepgs != bafhstat[dividx].af_oderpnr) {
        *retrelbf = NULL;
        *retmerbf = NULL;
        return FALSE;
    }
    *retrelbf = &bafhstat[dividx];
    *retmerbf = &bafhstat[MDIVMER_ARR_LMAX - 1];
    return TRUE;
}

sint_t mem_merpages_onmpgdesc(mpaflist_t *mpafh, mpdesc_t *freemsa, uint_t freepgs) {
    if (NULL == mpafh || NULL == freemsa || 1 > freepgs) {
        return 0;
    }
    if (MF_OLKTY_ODER != freemsa->mpd_indxflgs.mpf_olkty || NULL == freemsa->mpd_odlink) {
        panic("mm_merpages_opmsadsc err1\n");
    }
    mpdesc_t *fmend = (mpdesc_t *) freemsa->mpd_odlink;
    if (fmend < freemsa) {
        panic("mm_merpages_opmsadsc err2\n");
    }
    if (mpafh->af_oderpnr != freepgs || ((uint_t) (fmend - freemsa) + 1) != freepgs) {
        panic("mm_merpages_opmsadsc err3\n");
    }
    if (PAF_NO_ALLOC == freemsa->mpd_adrflgs.paf_alloc || 1 > freemsa->mpd_indxflgs.mpf_uindx) {
        panic("mm_merpages_opmsadsc err4\n");
    }
    if (PAF_NO_ALLOC == fmend->mpd_adrflgs.paf_alloc || 1 > fmend->mpd_indxflgs.mpf_uindx) {
        panic("mm_merpages_opmsadsc err5\n");
    }
    if (freemsa->mpd_indxflgs.mpf_uindx != fmend->mpd_indxflgs.mpf_uindx) {
        panic("mm_merpages_opmsadsc err6\n");
    }

    if (freemsa == fmend) {
        freemsa->mpd_indxflgs.mpf_uindx--;
        if (freemsa->mpd_indxflgs.mpf_uindx > 0) {
            return 1;
        }
        freemsa->mpd_adrflgs.paf_alloc = PAF_NO_ALLOC;
        freemsa->mpd_indxflgs.mpf_olkty = MF_OLKTY_BAFH;
        freemsa->mpd_odlink = mpafh;
        return 2;
    }

    freemsa->mpd_indxflgs.mpf_uindx--;
    fmend->mpd_indxflgs.mpf_uindx--;
    if (0 < freemsa->mpd_indxflgs.mpf_uindx) {
        return 1;
    }

    freemsa->mpd_adrflgs.paf_alloc = PAF_NO_ALLOC;
    freemsa->mpd_indxflgs.mpf_olkty = MF_OLKTY_ODER;
    freemsa->mpd_odlink = fmend;

    fmend->mpd_adrflgs.paf_alloc = PAF_NO_ALLOC;
    fmend->mpd_indxflgs.mpf_olkty = MF_OLKTY_BAFH;
    fmend->mpd_odlink = mpafh;
    return 2;
}

sint_t check_mpgblk_isfree(mpaflist_t *bafh, mpdesc_t *_1ms, mpdesc_t *_1me) {
    if (NULL == bafh || NULL == _1ms || NULL == _1me)
        return 0;

    if (_1me < _1ms)
        return 0;

    // check if _1me is free
    if (_1me->mpd_indxflgs.mpf_olkty != MF_OLKTY_BAFH || _1me->mpd_adrflgs.paf_alloc != PAF_NO_ALLOC ||
        _1me->mpd_indxflgs.mpf_uindx != 0 || (mpaflist_t *) _1me->mpd_odlink != bafh)
        return 0;

    // if _1ms == _1me no need to check any more
    if (_1ms == _1me)
        return 2;

    // check if _1ms is free
    if (_1ms->mpd_indxflgs.mpf_olkty != MF_OLKTY_ODER || _1ms->mpd_adrflgs.paf_alloc != PAF_NO_ALLOC ||
        _1ms->mpd_indxflgs.mpf_uindx != 0)
        return 0;

    // check if _1ms's link is correct
    if (_1me != (mpdesc_t *) _1ms->mpd_odlink ||
        ((_1me->mpd_adrflgs.paf_paddr - _1ms->mpd_adrflgs.paf_paddr) != (uint_t) (_1me - _1ms)))
        return 0;

    return 2;
}

sint_t mem_cmpg2blk_isok(mpaflist_t *aflist, mpdesc_t *_1ms, mpdesc_t *_1me, mpdesc_t *_2ms, mpdesc_t *_2me) {
    if (NULL == aflist || NULL == _1ms || NULL == _1me || NULL == _2ms || NULL == _2me || _1ms == _2ms ||
        _1me == _2me) {
        return 0;
    }
    sint_t ret1s = 0, ret2s = 0;

    ret1s = check_mpgblk_isfree(aflist, _1ms, _1me);
    if (0 == ret1s)
        panic("mm_cmsa1blk_isok ret1s == 0\n");

    ret2s = check_mpgblk_isfree(aflist, _2ms, _2me);
    if (0 == ret2s)
        panic("mm_cmsa1blk_isok ret2s == 0\n");

    if (2 != ret1s || 2 != ret2s)
        return 0;

    if (_1ms < _2ms && _1me < _2me) {
        if ((_1me + 1) != _2ms || ((_1me->mpd_adrflgs.paf_paddr + 1) != _2ms->mpd_adrflgs.paf_paddr))
            return 1;

        return 2;
    }

    if (_1ms > _2ms && _1me > _2me) {
        if ((_2me + 1) != _1ms || ((_2me->mpd_adrflgs.paf_paddr + 1) != _1ms->mpd_adrflgs.paf_paddr)) {
            return 1;
        }

        return 4;
    }

    return 0;
}

bool_t clear_2mpgolflg(mpaflist_t *bafh, mpdesc_t *_1ms, mpdesc_t *_1me, mpdesc_t *_2ms, mpdesc_t *_2me) {
    if (NULL == bafh || NULL == _1ms || NULL == _1me || NULL == _2ms || NULL == _2me)
        return FALSE;

    if (_1ms == _2ms || _1me == _2me)
        return FALSE;

    _1me->mpd_indxflgs.mpf_olkty = MF_OLKTY_INIT;
    _1me->mpd_odlink = NULL;

    _2ms->mpd_indxflgs.mpf_olkty = MF_OLKTY_INIT;
    _2ms->mpd_odlink = NULL;

    _1ms->mpd_indxflgs.mpf_olkty = MF_OLKTY_ODER;
    _1ms->mpd_odlink = _2me;

    _2me->mpd_indxflgs.mpf_olkty = MF_OLKTY_BAFH;
    _2me->mpd_odlink = bafh;
    return TRUE;
}

bool_t chek_cl2molkflg(mpaflist_t *bafh, mpdesc_t *_1ms, mpdesc_t *_1me, mpdesc_t *_2ms, mpdesc_t *_2me) {
    if (NULL == bafh || NULL == _1ms || NULL == _1me || NULL == _2ms || NULL == _2me)
        return FALSE;

    if (_1ms == _2ms || _1me == _2me)
        return FALSE;

    if (((uint_t) (_2me - _1ms) + 1) != bafh->af_oderpnr)
        return FALSE;

    if (_1ms == _1me && _2ms == _2me) {
        if (MF_OLKTY_ODER != _1ms->mpd_indxflgs.mpf_olkty || (mpdesc_t *) _1ms->mpd_odlink != _2me) {
            return FALSE;
        }
        if (MF_OLKTY_BAFH != _2me->mpd_indxflgs.mpf_olkty || (mpaflist_t *) _2me->mpd_odlink != bafh) {
            return FALSE;
        }
        return TRUE;
    }

    if (MF_OLKTY_ODER != _1ms->mpd_indxflgs.mpf_olkty || (mpdesc_t *) _1ms->mpd_odlink != _2me) {
        return FALSE;
    }
    if (MF_OLKTY_INIT != _1me->mpd_indxflgs.mpf_olkty || NULL != _1me->mpd_odlink) {
        return FALSE;
    }
    if (MF_OLKTY_INIT != _2ms->mpd_indxflgs.mpf_olkty || NULL != _2ms->mpd_odlink) {
        return FALSE;
    }
    if (MF_OLKTY_BAFH != _2me->mpd_indxflgs.mpf_olkty || (mpaflist_t *) _2me->mpd_odlink != bafh) {
        return FALSE;
    }
    return TRUE;
}

sint_t mem_find_cmsa2blk(mpaflist_t *tmpbf, mpdesc_t **retmstart, mpdesc_t **retmend) {
    if (NULL == tmpbf || NULL == retmstart || NULL == retmend)
        return 0;

    mpdesc_t *freemstat = *retmstart;
    mpdesc_t *freemend = *retmend;
    if (1 > tmpbf->af_fobjnr)
        return 1;

    list_t *tmplst = NULL;
    mpdesc_t *tmpmsa = NULL, *blkms = NULL, *blkme = NULL;
    sint_t rets = 0;
    list_for_each(tmplst, &tmpbf->af_frelist) {
        tmpmsa = list_entry(tmplst, mpdesc_t, mpd_list);
        rets = mem_cmpg2blk_isok(tmpbf, freemstat, freemend, tmpmsa, &tmpmsa[tmpbf->af_oderpnr - 1]);
        if (2 == rets || 4 == rets) {
            blkms = tmpmsa;
            blkme = &tmpmsa[tmpbf->af_oderpnr - 1];
            list_del(&tmpmsa->mpd_list);
            tmpbf->af_fobjnr--;
            tmpbf->af_mobjnr--;
            break;
        }
    }

    if (0 == rets || 1 == rets)
        return 1;

    if (2 == rets) {
        if (clear_2mpgolflg(tmpbf + 1, freemstat, freemend, blkms, blkme) == TRUE) {
            if (chek_cl2molkflg(tmpbf + 1, freemstat, freemend, blkms, blkme) == FALSE) {
                panic("chek_cl2molkflg err1\n");
            }
            *retmstart = freemstat;
            *retmend = blkme;
            return 2;
        }
        return 0;
    }

    if (4 == rets) {
        if (clear_2mpgolflg(tmpbf + 1, blkms, blkme, freemstat, freemend) == TRUE) {
            if (chek_cl2molkflg(tmpbf + 1, blkms, blkme, freemstat, freemend) == FALSE) {
                panic("chek_cl2molkflg err2\n");
            }
            *retmstart = blkms;
            *retmend = freemend;
            return 2;
        }

        return 0;
    }
    return 0;
}

bool_t mpobf_add_msadsc(mpaflist_t *bafhp, mpdesc_t *freemstat, mpdesc_t *freemend) {
    if (NULL == bafhp || NULL == freemstat || NULL == freemend) {
        return FALSE;
    }
    if (freemend < freemstat) {
        return FALSE;
    }
    if (bafhp->af_oderpnr != ((uint_t) (freemend - freemstat) + 1)) {
        return FALSE;
    }
    if ((~0UL) <= bafhp->af_fobjnr || (~0UL) <= bafhp->af_mobjnr) {
        panic("(~0UL)<=bafhp->af_fobjnr\n");
    }
    freemstat->mpd_indxflgs.mpf_olkty = MF_OLKTY_ODER;
    freemstat->mpd_odlink = freemend;
    freemend->mpd_indxflgs.mpf_olkty = MF_OLKTY_BAFH;
    freemend->mpd_odlink = bafhp;
    list_add(&freemstat->mpd_list, &bafhp->af_frelist);
    bafhp->af_fobjnr++;
    bafhp->af_mobjnr++;
    bafhp->af_freindx++;
    return TRUE;
}

bool_t mem_merpages_onmpaflist(mpdesc_t *freempg, uint_t freepgs, mpaflist_t *relbf, mpaflist_t *merbf) {
    sint_t rets = 0;
    mpdesc_t *mnxs = freempg, *mnxe = &freempg[freepgs - 1];
    mpaflist_t *tmpbf = relbf;
    for (; tmpbf < merbf; tmpbf++) {
        rets = mem_find_cmsa2blk(tmpbf, &mnxs, &mnxe);
        if (1 == rets) {
            break;
        }
        if (0 == rets) {
            panic("mm_find_cmsa2blk retn 0\n");
        }
    }

    if (mpobf_add_msadsc(tmpbf, mnxs, mnxe) == FALSE) {
        return FALSE;
    }
    return TRUE;
}

bool_t mem_merpages_onmarea(memarea_t *marea, mpdesc_t *freempg, uint_t freepgs) {
    if (NULL == marea || NULL == freempg || 1 > freepgs)
        return FALSE;

    mpaflist_t *prcbf = NULL;
    sint_t pocs = 0;

    // TODO process
    if (MA_TYPE_PROC == marea->ma_type) {
        // prcbf = &marea->ma_mdmdata.dm_onelst;
        // pocs = mm_merpages_opmsadsc(prcbf, freemsa, freepgs);
        // if (2 == pocs)
        // {
        //     if (mpobf_add_msadsc(prcbf, freemsa, &freemsa[freepgs - 1]) == FALSE)
        //     {
        //         panic("mm_merpages_onmarea proc memarea merge fail\n");
        //     }
        //     mm_update_memarea(malckp, freepgs, 1);
        //     mm_update_memmgrob(freepgs, 1);
        //     return TRUE;
        // }
        // if (1 == pocs)
        // {
        //     return TRUE;
        // }
        return FALSE;
    }

    mpaflist_t *retrelbf = NULL, *retmerbf = NULL;

    if (!onpgs_ret_mpafhlst(marea, freepgs, &retrelbf, &retmerbf))
        return FALSE;

    if (retrelbf == NULL || retmerbf == NULL)
        return FALSE;

    sint_t mopms = mem_merpages_onmpgdesc(retrelbf, freempg, freepgs);

    // the page is still hold by others don't free
    if (mopms == 1)
        return TRUE;

    if (mopms != 2)
        return FALSE;

    if (mem_merpages_onmpaflist(freempg, freepgs, retrelbf, retmerbf)) {
        update_memarea(marea, freepgs, 1);
        update_memmgrob(freepgs, 1);
        return TRUE;
    }

    return FALSE;
}

bool_t memory_merge_pages_core(memarea_t *marea, mpdesc_t *freemsa, uint_t freepgs) {
    if (scan_freemsa_isok(freemsa, freepgs) == FALSE) {
        return FALSE;
    }
    bool_t rets = FALSE;
    cpuflg_t cpuflg;

    spinlock_cli(&marea->ma_lock, &cpuflg);

    rets = mem_merpages_onmarea(marea, freemsa, freepgs);

    spinunlock_sti(&marea->ma_lock, &cpuflg);

    return rets;
}

// 释放内存页面框架函数
bool_t mem_merpages_framework(memgrob_t *mmobjp, mpdesc_t *freemsa, uint_t freepgs) {
    // 获取要释放msadsc_t结构所在的内存区
    memarea_t *marea = memarea_by_page(mmobjp, freemsa, freepgs);
    if (NULL == marea)
        return FALSE;

    // 释放内存页面的核心函数
    bool_t rets = memory_merge_pages_core(marea, freemsa, freepgs);
    if (FALSE == rets)
        return FALSE;

    return rets;
}

// 释放内存页面接口

// mmobjp->内存管理数据结构指针
// freemsa->释放内存页面对应的首个msadsc_t结构指针
// freepgs->请求释放的内存页面数
bool_t memory_merge_pages(memgrob_t *mmobjp, mpdesc_t *freemsa, uint_t freepgs) {
    if (NULL == mmobjp || NULL == freemsa || 1 > freepgs) {
        return FALSE;
    }
    // 调用释放内存页面的框架函数
    return mem_merpages_framework(mmobjp, freemsa, freepgs);
}