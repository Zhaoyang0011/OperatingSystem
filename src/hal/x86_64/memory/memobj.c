#include <console.h>
#include <hal/memory/memdivmer.h>
#include <hal/memory/memgrob.h>
#include <hal/memory/memobject.h>
#include <memlayout.h>
#include <type.h>
#include "../io.h"

void freekmemobj_init(freekmemobj_t *tmpfoh, uint_t stus, void *start) {
    list_init(&tmpfoh->oh_list);
    tmpfoh->oh_stat = start;
    tmpfoh->oh_stus = stus;
}

void kmopglist_init(kmopglist_t *initp, uint_t pnr) {
    initp->mpl_msanr = 0;
    initp->mpl_ompnr = pnr;
    list_init(&initp->mpl_list);
}

void kmopglist_container_init(kmopglist_container_t *initp) {
    for (uint_t i = 0; i < MSCLST_MAX; i++) {
        kmopglist_init(&initp->mc_lst[i], 1UL << i);
    }
    initp->mc_mpgnr = 0;
    list_init(&initp->mc_list);
    list_init(&initp->mc_kmobinlst);
    initp->mc_kmobinpnr = 0;
}

void kmomgr_init(kmomgr_t *initp) {
    list_init(&initp->so_list);
    spinlock_init(&initp->so_lock);
    initp->so_stus = 0;
    initp->so_flgs = 0;
    initp->so_vstat = 0;
    initp->so_vend = 0;
    initp->so_objsz = 0;
    initp->so_objrelsz = 0;
    initp->so_mobjnr = 0;
    initp->so_fobjnr = 0;
    list_init(&initp->so_frelst);
    list_init(&initp->so_alclst);
    list_init(&initp->so_mextlst);
    initp->so_mextnr = 0;
    kmopglist_container_init(&initp->so_mc);
    initp->so_privp = NULL;
    initp->so_extdp = NULL;
}

void kmomgrext_init(kmomgrext_t *initp, addr_t vstat, addr_t vend, kmomgr_t *pmomgr) {
    list_init(&initp->mt_list);
    initp->mt_vstat = vstat;
    initp->mt_vend = vend;
    initp->mt_kmsb = pmomgr;
}

void kmomgr_list_init(kmomgrlist_t *initp, size_t mobsz) {
    list_init(&initp->ol_emplst);
    initp->ol_cache = NULL;
    initp->ol_emnr = 0;
    initp->ol_sz = mobsz;
}

void init_memory_object_manager() {
    kmomgrlist_header_t *initp = &memgrob.mo_mobmgr;
    size_t mobsz = 32;
    spinlock_init(&initp->ks_lock);
    list_init(&initp->ks_tclst);
    initp->ks_tcnr = 0;
    initp->ks_msobnr = 0;
    initp->ks_msobche = NULL;
    for (uint_t i = 0; i < KOBLST_MAX; i++) {
        kmomgr_list_init(&initp->ks_msoblst[i], mobsz);
        mobsz += 32;
    }
}

KLINE sint_t retn_mscidx(uint_t pages) {
    sint_t pbits = search_64rlbits((uint_t) pages) - 1;
    if (pages & (pages - 1)) {
        pbits++;
    }
    return pbits;
}

kmomgrlist_t *onmsz_ret_kmomgrlist(kmomgrlist_header_t *pmomgrh, size_t size) {
    if (pmomgrh == NULL || size < 1 || size > 2048)
        return NULL;

    for (int i = 0; i < KOBLST_MAX; ++i) {
        if (size < pmomgrh->ks_msoblst[i].ol_sz)
            return &pmomgrh->ks_msoblst[i];
    }

    return NULL;
}

bool_t kmomgr_isok(kmomgr_t *momgrp, size_t msz) {
    if (momgrp == NULL || msz < 1)
        return FALSE;

    if (momgrp->so_fobjnr > 0 && momgrp->so_objsz > msz)
        return TRUE;

    return FALSE;
}

kmomgr_t *onmgrlist_ret_kmomgr(kmomgrlist_t *pmgrlist, size_t msz) {
    if (pmgrlist == NULL || pmgrlist->ol_sz < msz)
        return NULL;

    if (kmomgr_isok(pmgrlist->ol_cache, msz))
        return pmgrlist->ol_cache;

    if (pmgrlist->ol_emnr > 0) {
        list_t *tmplst = NULL;
        list_for_each(tmplst, &pmgrlist->ol_emplst) {
            kmomgr_t *momgrp = list_entry(tmplst, kmomgr_t, so_list);
            if (kmomgr_isok(momgrp, msz))
                return momgrp;
        }
    }

    return NULL;
}

kmomgr_t *_create_kmomgr_onmpg(kmomgr_t *pkmomgr, size_t objsz, addr_t vadrs, addr_t vadre, mpdesc_t *mpgs,
                               uint_t relpnr) {
    if (NULL == pkmomgr || 1 > objsz || NULL == vadrs || NULL == vadre || NULL == mpgs || 1 > relpnr)
        return NULL;
    if (objsz < sizeof(freekmemobj_t))
        return NULL;
    if ((vadre - vadrs + 1) < PAGE_SIZE)
        return NULL;
    if ((vadre - vadrs + 1) <= (sizeof(kmomgr_t) + sizeof(freekmemobj_t)))
        return NULL;

    kmomgr_init(pkmomgr);

    pkmomgr->so_vstat = vadrs;
    pkmomgr->so_vend = vadre;
    pkmomgr->so_objsz = objsz;

    list_add(&mpgs->mpd_list, &pkmomgr->so_mc.mc_kmobinlst);

    freekmemobj_t *fohstat = (freekmemobj_t *) (pkmomgr + 1), *fohend = (freekmemobj_t *) vadre;

    uint_t ap = (uint_t) fohstat;
    freekmemobj_t *tmpfoh = (freekmemobj_t *) ap;
    for (; tmpfoh < fohend;) {
        if ((ap + (uint_t) pkmomgr->so_objsz) <= (uint_t) vadre) {
            freekmemobj_init(tmpfoh, 0, (void *) tmpfoh);
            list_add(&tmpfoh->oh_list, &pkmomgr->so_frelst);
            pkmomgr->so_mobjnr++;
            pkmomgr->so_fobjnr++;
        }
        ap += (uint_t) pkmomgr->so_objsz;
        tmpfoh = (freekmemobj_t *) ((uint_t) ap);
    }
    return pkmomgr;
}

bool_t kmomgrlist_add_kmpmgr(kmomgrlist_t *koblp, kmomgr_t *kmsp) {
    if (koblp == NULL || kmsp == NULL)
        return FALSE;

    if (kmsp->so_objsz > koblp->ol_sz)
        return FALSE;

    list_add(&kmsp->so_list, &koblp->ol_emplst);
    koblp->ol_emnr++;
    return TRUE;
}

kmomgr_t *create_init_kmomgr(kmomgrlist_header_t *pmomgrh, kmomgrlist_t *koblp) {
    if (pmomgrh == NULL || koblp == NULL)
        return NULL;

    size_t objsz = koblp->ol_sz;

    uint_t pages = 1;
    uint_t relpnr = 0;
    if (128 < objsz)
        pages = 2;
    if (512 < objsz)
        pages = 4;

    mpdesc_t *mpgs = memory_divide_pages(&memgrob, pages, &relpnr, MA_TYPE_KRNL, DMF_RELDIV);
    if (NULL == mpgs)
        return NULL;

    if (0 == relpnr) {
        panic("_create_kmomgr memory_divide_pages fail\n");
        return NULL;
    }

    uint64_t phyadr = mpgs->mpd_addr & ~(PAGE_SIZE - 1);
    uint64_t phyade = phyadr + (relpnr << PAGE_SHR) - 1;
    addr_t vadrs = P2V((addr_t) phyadr);
    addr_t vadre = P2V((addr_t) phyade);

    kmomgr_t *kmsp = _create_kmomgr_onmpg((kmomgr_t *) vadrs, koblp->ol_sz, vadrs, vadre, mpgs, relpnr);

    if (kmsp == NULL) {
        if (memory_merge_pages(&memgrob, mpgs, relpnr) == FALSE)
            panic("_create_kmomgr memory_merge_pages fail\n");
        return NULL;
    }
    if (kmomgrlist_add_kmpmgr(koblp, kmsp) == FALSE) {
        panic("_create_kmomgr b kmsob_add_koblst FALSE\n");
    }

    pmomgrh->ks_msobnr++;
    return kmsp;
}

uint_t scan_kmomgr_objnr(kmomgr_t *kmsp) {
    if (kmsp->so_fobjnr > 1 && !list_is_empty_careful(&kmsp->so_frelst))
        return kmsp->so_fobjnr;
    return 0;
}

bool_t kmomgr_extern_pages(kmomgr_t *pmomgr) {
    if (pmomgr == NULL)
        return FALSE;

    if ((~0UL) <= pmomgr->so_mobjnr || (~0UL) <= pmomgr->so_mextnr || (~0UL) <= pmomgr->so_fobjnr) {
        return FALSE;
    }
    mpdesc_t *mpgs = NULL;
    uint_t relpnr = 0;
    uint_t pages = 1;
    if (128 < pmomgr->so_objsz) {
        pages = 2;
    }
    if (512 < pmomgr->so_objsz) {
        pages = 4;
    }

    mpgs = memory_divide_pages(&memgrob, pages, &relpnr, MA_TYPE_KRNL, DMF_RELDIV);
    if (NULL == mpgs)
        return FALSE;

    if (0 == relpnr)
        panic("kmomgr_extern_pages memory_divide_pages fail\n");

    if (relpnr != pages) {
        memory_merge_pages(&memgrob, mpgs, relpnr);
        panic("kmomgr_extern_pages memory_divide_pages number error\n");
    }

    uint64_t phyadr = mpgs->mpd_addr & ~(PAGE_SIZE - 1);
    uint64_t phyade = phyadr + (relpnr << PAGE_SHR) - 1;
    addr_t vadrs = P2V((addr_t) phyadr);
    addr_t vadre = P2V((addr_t) phyade);

    uint_t mscidx = retn_mscidx(relpnr);
    list_add(&mpgs->mpd_list, &pmomgr->so_mc.mc_lst[mscidx].mpl_list);

    kmomgrext_t *pext = (kmomgrext_t *) vadrs;
    kmomgrext_init(pext, vadrs, vadre, pmomgr);

    freekmemobj_t *fmobj = (freekmemobj_t *) (pext + 1);
    for (; fmobj < (freekmemobj_t *) vadre;) {
        addr_t temp = (addr_t) fmobj;
        if (temp + pmomgr->so_objsz < vadre) {
            freekmemobj_init(fmobj, 0, (void *) fmobj);
            list_add(&fmobj->oh_list, &pmomgr->so_frelst);
            pmomgr->so_mobjnr++;
            pmomgr->so_fobjnr++;
        }
        fmobj = (freekmemobj_t *) (temp + pmomgr->so_objsz);
    }
    list_add(&pext->mt_list, &pmomgr->so_mextlst);
    pmomgr->so_mobjnr++;
    return TRUE;
}

bool_t scan_kmomgr_isok(kmomgr_t *kmsp, size_t msz) {
    if (NULL == kmsp || 1 > msz) {
        return FALSE;
    }
    if (1 > kmsp->so_fobjnr || 1 > kmsp->so_mobjnr) {
        return FALSE;
    }
    if (msz > kmsp->so_objsz) {
        return FALSE;
    }
    if ((kmsp->so_vend - kmsp->so_vstat + 1) < PAGE_SIZE ||
        (kmsp->so_vend - kmsp->so_vstat + 1) < (addr_t) (sizeof(kmomgr_t) + sizeof(freekmemobj_t))) {
        return FALSE;
    }
    if (list_is_empty_careful(&kmsp->so_frelst) == TRUE) {
        return FALSE;
    }
    return TRUE;
}

void *onkmomgr_ret_memobj(kmomgr_t *kmsp, size_t msz) {
    if (NULL == kmsp) {
        return NULL;
    }
    void *retptr = NULL;
    cpuflg_t cpuflg;
    spinlock_cli(&kmsp->so_lock, &cpuflg);

    if (scan_kmomgr_objnr(kmsp) < 1) {
        if (kmomgr_extern_pages(kmsp) == FALSE) {
            retptr = NULL;
            goto ret_step;
        }
    }

    if (!scan_kmomgr_isok(kmsp, msz))
        goto ret_step;

    freekmemobj_t *fobh = list_entry(kmsp->so_frelst.next, freekmemobj_t, oh_list);
    list_del(&fobh->oh_list);
    kmsp->so_fobjnr--;
    retptr = fobh;

    ret_step:
    spinunlock_sti(&kmsp->so_lock, &cpuflg);
    return retptr;
}

KLINE void kmomgrhead_update_cache(kmomgrlist_header_t *pmomgrh, kmomgr_t *kmsp) {
    pmomgrh->ks_msobche = kmsp;
}

// 分配内存对象的核心函数
void *mobj_alloc_core(size_t msz) {
    kmomgrlist_header_t *pmomgrh = &memgrob.mo_mobmgr;

    cpuflg_t cpuflg;
    spinlock_cli(&pmomgrh->ks_lock, &cpuflg);

    void *retptr = NULL;

    kmomgrlist_t *pmgrlist = onmsz_ret_kmomgrlist(pmomgrh, msz);
    if (NULL == pmgrlist) {
        retptr = NULL;
        goto out;
    }

    kmomgr_t *kmsp = onmgrlist_ret_kmomgr(pmgrlist, msz);
    if (kmsp == NULL) {
        kmsp = create_init_kmomgr(pmomgrh, pmgrlist);
        if (kmsp == NULL)
            goto out;
    }

    retptr = onkmomgr_ret_memobj(kmsp, msz);
    if (retptr == NULL)
        goto out;

    // update cache
    kmomgrhead_update_cache(pmomgrh, kmsp);
    out:
    spinunlock_sti(&pmomgrh->ks_lock, &cpuflg);
    return retptr;
}

// 内存对象分配接口
void *mobj_alloc(size_t msz) {
    // 对于小于1 或者 大于2048字节的大小不支持 直接返回NULL表示失败
    if (1 > msz || 2048 < msz) {
        return NULL;
    }
    // 调用核心函数
    return mobj_alloc_core(msz);
}