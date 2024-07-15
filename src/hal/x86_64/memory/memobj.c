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
//    list_init(&initp->mc_list);
  initp->mc_mpdescptr = NULL;
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
  sint_t pbits = search_64rlbits((uint_t)pages) - 1;
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

kmomgr_t *_create_kmomgr_onmpg(mpdesc_t *mpgs, uint_t relpnr, size_t objsz) {
  if (NULL == mpgs || 1 > relpnr)
    return NULL;
  if (objsz < sizeof(freekmemobj_t))
    return NULL;

  uint64_t phyadr = mpgs->mpd_addr & ~(PAGE_SIZE - 1);
  uint64_t phyade = phyadr + (relpnr << PAGE_SHR) - 1;
  addr_t vadrs = P2V((addr_t)phyadr);
  addr_t vadre = P2V((addr_t)phyade);

  uint64_t adr_len = vadre - vadrs + 1;

  if (adr_len < PAGE_SIZE - 1) {
    return NULL;
  }

  kmomgr_t *pkmomgr = (kmomgr_t *)vadrs;

  kmomgr_init(pkmomgr);

  pkmomgr->so_vstat = vadrs;
  pkmomgr->so_vend = vadre;
  pkmomgr->so_objsz = objsz;

  pkmomgr->so_mc.mc_mpdescptr = mpgs;
  pkmomgr->so_mc.mc_kmobinpnr = relpnr;

  freekmemobj_t *fohstat = (freekmemobj_t *)(pkmomgr + 1);
  freekmemobj_t *fohend = (freekmemobj_t *)vadre;

  uint_t ap = (uint_t)fohstat;
  freekmemobj_t *tmpfoh = (freekmemobj_t *)ap;
  for (; tmpfoh < fohend;) {
    if ((ap + (uint_t)pkmomgr->so_objsz) <= (uint_t)vadre) {
      freekmemobj_init(tmpfoh, 0, (void *)tmpfoh);
      list_add(&tmpfoh->oh_list, &pkmomgr->so_frelst);
      pkmomgr->so_mobjnr++;
      pkmomgr->so_fobjnr++;
    }
    ap += (uint_t)pkmomgr->so_objsz;
    tmpfoh = (freekmemobj_t *)((uint_t)ap);
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

static kmomgr_t *create_init_kmomgr(kmomgrlist_header_t *pmomgrh, kmomgrlist_t *koblp) {
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

  kmomgr_t *kmsp = _create_kmomgr_onmpg(mpgs, relpnr, koblp->ol_sz);

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

static uint_t scan_kmomgr_objnr(kmomgr_t *kmsp) {
  if (kmsp->so_fobjnr > 1 && !list_is_empty_careful(&kmsp->so_frelst))
    return kmsp->so_fobjnr;
  return 0;
}

static bool_t kmomgr_extern_pages(kmomgr_t *pmomgr) {
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
  addr_t vadrs = P2V((addr_t)phyadr);
  addr_t vadre = P2V((addr_t)phyade);

  uint_t mscidx = retn_mscidx(relpnr);
  list_add(&mpgs->mpd_list, &pmomgr->so_mc.mc_lst[mscidx].mpl_list);
  pmomgr->so_mc.mc_lst[mscidx].mpl_msanr++;

  kmomgrext_t *pext = (kmomgrext_t *)vadrs;
  kmomgrext_init(pext, vadrs, vadre, pmomgr);

  freekmemobj_t *fmobj = (freekmemobj_t *)(pext + 1);
  for (; fmobj < (freekmemobj_t *)vadre;) {
    addr_t temp = (addr_t)fmobj;
    if (temp + pmomgr->so_objsz < vadre) {
      freekmemobj_init(fmobj, 0, (void *)fmobj);
      list_add(&fmobj->oh_list, &pmomgr->so_frelst);
      pmomgr->so_mobjnr++;
      pmomgr->so_fobjnr++;
      pext->mt_mobjnr++;
    }
    fmobj = (freekmemobj_t *)(temp + pmomgr->so_objsz);
  }

  // Add kmomgrext_t to kmomgr_t
  list_add(&pext->mt_list, &pmomgr->so_mextlst);
  pmomgr->so_mobjnr++;
  return TRUE;
}

static bool_t scan_kmomgr_isok(kmomgr_t *kmsp, size_t msz) {
  if (1 > kmsp->so_fobjnr || 1 > kmsp->so_mobjnr) {
    return FALSE;
  }
  if (msz > kmsp->so_objsz) {
    return FALSE;
  }
  if ((kmsp->so_vend - kmsp->so_vstat + 1) < PAGE_SIZE ||
      (kmsp->so_vend - kmsp->so_vstat + 1) < (addr_t)(sizeof(kmomgr_t) + sizeof(freekmemobj_t))) {
    return FALSE;
  }
  if (list_is_empty_careful(&kmsp->so_frelst) == TRUE) {
    return FALSE;
  }
  return TRUE;
}

static void *onkmomgr_ret_memobj(kmomgr_t *kmsp, size_t msz) {
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
static void *mobj_alloc_core(size_t msz) {
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
void *kmobj_alloc(size_t size) {
  // 对于小于1 或者 大于2048字节的大小不支持 直接返回NULL表示失败
  if (1 > size || 2048 < size) {
    return NULL;
  }
  // 调用核心函数
  return mobj_alloc_core(size);
}

KLINE bool_t ismobj_from_kmomgrext(kmomgrext_t *pmomgrext, void *addr) {
  if ((void *)pmomgrext->mt_vstat < addr && (void *)pmomgrext->mt_vend > addr)
    return TRUE;
  return FALSE;
}

static bool_t ismobj_from_kmomgr(kmomgr_t *pmomgr, void *addr) {
  if (pmomgr == NULL)
    return FALSE;

  if (pmomgr->so_vstat < addr && pmomgr->so_vend > addr)
    return TRUE;

  list_t *temp = NULL;
  list_for_each(temp, &pmomgr->so_mextlst) {
    kmomgrext_t *pext = list_entry(temp, kmomgrext_t, mt_list);
    if (pext->mt_kmsb != pmomgr)
      panic("kmomgrext is loaded on wrong kmomgr!");
    if (ismobj_from_kmomgrext(pext, addr))
      return TRUE;
  }
  return FALSE;
}

static kmomgr_t *onkmomgrlist_retfree_mobj(kmomgrlist_t *pmgrlist, void *faddr) {
  if (ismobj_from_kmomgr(pmgrlist->ol_cache, faddr))
    return pmgrlist->ol_cache;

  list_t *temp = NULL;
  list_for_each(temp, &pmgrlist->ol_emplst) {
    kmomgr_t *pmgr = list_entry(temp, kmomgr_t, so_list);
    if (ismobj_from_kmomgr(pmgr, faddr))
      return pmgr;
  }
  return FALSE;
}

static kmomgr_t *try_free_onkmomgrlist(kmomgrlist_t *pmgrlist, void *addr) {
  kmomgr_t *pmomgr = onkmomgrlist_retfree_mobj(pmgrlist, addr);
  if (NULL == pmomgr) {
    return NULL;
  }

  cpuflg_t flg = 0;
  spinlock_cli(&pmomgr->so_lock, &flg);

  freekmemobj_t *obhp = (freekmemobj_t *)addr;
  freekmemobj_init(obhp, 0, obhp);
  list_add(&obhp->oh_list, &pmomgr->so_frelst);
  pmomgr->so_fobjnr++;

  spinunlock_sti(&pmomgr->so_lock, &flg);

  return pmomgr;
}

static bool_t try_free_kmomgr(kmomgrlist_header_t *kmobmgrp, kmomgrlist_t *koblp, kmomgr_t *kmsp) {
  if (kmsp->so_mobjnr < kmsp->so_fobjnr)
    panic("kmomogr_t free object bigger than total!");
  if (kmsp->so_fobjnr < kmsp->so_mobjnr)
    return FALSE;

  list_t *tmplst = NULL;
  mpdesc_t *msa = NULL;
  kmopglist_t *mscp = kmsp->so_mc.mc_lst;
  list_del(&kmsp->so_list);
  koblp->ol_emnr--;
  kmobmgrp->ks_msobnr--;
  for (uint_t j = 0; j < MSCLST_MAX; j++) {
    if (0 < mscp[j].mpl_msanr) {
      list_for_each_head_dell(tmplst, &mscp[j].mpl_list) {
        msa = list_entry(tmplst, mpdesc_t, mpd_list);
        list_del(&msa->mpd_list);
        if (!memory_merge_pages(&memgrob, msa, (uint_t)mscp[j].mpl_ompnr)) {
          panic("_destroy_kmsob_core mm_merge_pages FALSE2\n");
        }
      }
    }
  }

  if (!memory_merge_pages(&memgrob, kmsp->so_mc.mc_mpdescptr, kmsp->so_mc.mc_kmobinpnr))
    panic("free kmomgr merge page error!");
  return TRUE;
}

static bool_t kmobj_free_core(void *addr, size_t size) {
  kmomgrlist_header_t *pmomgrh = &memgrob.mo_mobmgr;

  cpuflg_t cpuflg;
  spinlock_cli(&pmomgrh->ks_lock, &cpuflg);

  bool_t ret = FALSE;
  kmomgrlist_t *pmgrlist = onmsz_ret_kmomgrlist(pmomgrh, size);
  if (NULL == pmgrlist) {
    ret = FALSE;
    goto out;
  }

  kmomgr_t *ksmp = try_free_onkmomgrlist(pmgrlist, addr);
  if (ksmp == NULL) {
    ret = FALSE;
    goto out;
  }

  try_free_kmomgr(pmomgrh, pmgrlist, ksmp);
  ret = TRUE;
out:
  spinunlock_sti(&pmomgrh->ks_lock, &cpuflg);
  return ret;
}

bool_t kmobj_free(void *addr, size_t size) {
  // 对于小于1 或者 大于2048字节的大小不支持 直接返回NULL表示失败
  if (addr == NULL || size == 0 || size > 2048)
    return FALSE;
  // 调用核心函数
  return kmobj_free_core(addr, size);
}