#include <console.h>
#include <hal/memory/memdivmer.h>
#include <hal/memory/memgrob.h>
#include <hal/memory/memobject.h>
#include <memlayout.h>
#include <type.h>

void kmomgr_list_init(kmomgr_list_t *initp, size_t mobsz) {
    list_init(&initp->ol_emplst);
    initp->ol_cache = NULL;
    initp->ol_emnr = 0;
    initp->ol_sz = mobsz;
}

void init_memory_object_manager() {
    kmomgr_header_t *initp = &memgrob.mo_mobmgr;
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
    return;
}

kmomgr_list_t *onmsz_ret_kmomgrlist(kmomgr_header_t *pmomgrh, size_t size) {
    if (pmomgrh == NULL || size < 1 || size > 2048)
        return NULL;

    size_t idx = size / 32;

    return &pmomgrh->ks_msoblst[idx];
}

bool_t kmomgr_isok(kmomgr_t *momgrp, size_t msz) {
    if (momgrp == NULL || msz < 0)
        return FALSE;

    if (momgrp->so_fobjnr > 0 && momgrp->so_objsz > msz)
        return TRUE;

    return FALSE;
}

kmomgr_t *onmgrlist_ret_kmomgr(kmomgr_list_t *pmgrlist, size_t msz) {
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

void kmopglist_init(kmopglist_t *initp, uint_t pnr) {
    initp->mpl_msanr = 0;
    initp->mpl_ompnr = pnr;
    list_init(&initp->mpl_list);
    return;
}

void kmopglist_container_init(kmopglist_container_t *initp) {
    for (uint_t i = 0; i < MSCLST_MAX; i++) {
        kmopglist_init(&initp->mc_lst[i], 1UL << i);
    }
    initp->mc_mpgnr = 0;
    list_init(&initp->mc_list);
    list_init(&initp->mc_kmobinlst);
    initp->mc_kmobinpnr = 0;
    return;
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
    return;
}

void freekmemobj_init(freekmemobj_t *tmpfoh, uint_t stus, void *start) {
    list_init(&tmpfoh->oh_list);
    tmpfoh->oh_stat = start;
    tmpfoh->oh_stus = stus;
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

bool_t kmpmgr_add_kmomgrlist(kmomgr_list_t *koblp, kmomgr_t *kmsp) {
    if (koblp == NULL || kmsp == NULL)
        return FALSE;

    if (kmsp->so_objsz > koblp->ol_sz)
        return FALSE;

    list_add(&kmsp->so_list, &koblp->ol_emplst);
    koblp->ol_emnr++;
    return TRUE;
}

kmomgr_t *create_kmomgr(kmomgr_header_t *pmomgrh, kmomgr_list_t *koblp) {
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

    uint64_t phyadr = mpgs->mpd_addr & PAGE_SIZE;
    uint64_t phyade = phyadr + (relpnr << PAGE_SHR) - 1;
    addr_t vadrs = P2V((addr_t) phyadr);
    addr_t vadre = P2V((addr_t) phyade);

    kmomgr_t *kmsp = _create_kmomgr_onmpg((kmomgr_t *) vadrs, koblp->ol_sz, vadrs, vadre, mpgs, relpnr);

    if (kmsp == NULL) {
        if (memory_merge_pages(&memgrob, mpgs, relpnr) == FALSE)
            panic("_create_kmomgr memory_merge_pages fail\n");
        return NULL;
    }
    if (kmpmgr_add_kmomgrlist(koblp, kmsp) == FALSE) {
        panic("_create_kmomgr b kmsob_add_koblst FALSE\n");
    }

    pmomgrh->ks_msobnr++;
    return kmsp;
}

uint_t scan_kmomgr_objnr(kmomgr_t *kmsp) {
    if (kmsp->so_fobjnr > 1 && list_is_empty_careful(&kmsp->so_frelst))
        return kmsp->so_fobjnr;
    return 0;
}

bool_t kmomgr_extern_pages(kmomgr_t *kmsp) {
    if (kmsp == NULL)
        return FALSE;

    if ((~0UL) <= kmsp->so_mobjnr || (~0UL) <= kmsp->so_mextnr || (~0UL) <= kmsp->so_fobjnr) {
        return FALSE;
    }
    mpdesc_t *mpgs = NULL;
    uint_t relpnr = 0;
    uint_t pages = 1;
    if (128 < kmsp->so_objsz) {
        pages = 2;
    }
    if (512 < kmsp->so_objsz) {
        pages = 4;
    }

    mpgs = memory_divide_pages(&memgrob, pages, &relpnr, MA_TYPE_KRNL, DMF_RELDIV);
    if (NULL == mpgs)
        return FALSE;

    if (0 == relpnr)
        panic("kmomgr_extern_pages memory_divide_pages fail\n");

    uint64_t phyadr = mpgs->mpd_addr & PAGE_SIZE;
    uint64_t phyade = phyadr + (relpnr << PAGE_SHR) - 1;
    addr_t vadrs = P2V((addr_t) phyadr);
    addr_t vadre = P2V((addr_t) phyade);

    return TRUE;
}

void *onkmomgr_ret_memobj(kmomgr_t *kmsp) {
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
    // retptr = kmsob_new_opkmsob(kmsp, msz);

    ret_step:
    spinunlock_sti(&kmsp->so_lock, &cpuflg);
    return retptr;
}

KLINE void kmomgrhead_update_cache(kmomgr_header_t *pmomgrh, kmomgr_t *kmsp) {
    pmomgrh->ks_msobche = kmsp;
}

// 分配内存对象的核心函数
void *mobj_alloc_core(size_t msz) {
    kmomgr_header_t *pmomgrh = &memgrob.mo_mobmgr;

    cpuflg_t cpuflg;
    spinlock_cli(&pmomgrh->ks_lock, &cpuflg);

    void *retptr = NULL;

    kmomgr_list_t *pmgrlist = onmsz_ret_kmomgrlist(pmomgrh, msz);

    if (pmgrlist == NULL)
        goto out;

    kmomgr_t *kmsp = onmgrlist_ret_kmomgr(pmgrlist, msz);

    if (kmsp == NULL) {
        kmsp = create_kmomgr(pmomgrh, pmgrlist);
        if (kmsp == NULL)
            goto out;
    }

    retptr = onkmomgr_ret_memobj(kmsp);

    if (retptr != NULL)
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