#include "kernel/krlmempool.h"
#include "hal/memory/memdivmer.h"
#include "kernel/virmem.h"
#include "console.h"

addr_t mpdesc_ret_vaddr(mpdesc_t *retmsa) {
  if (retmsa == NULL || (retmsa->mpd_addr & PAGE_SIZE) > KRNL_MAP_VIRTADDRESS_END) {
    panic("phyadr_to_viradr err");
    return KRNL_ADDR_ERROR;
  }
  return retmsa->mpd_addr & PAGE_SIZE;
}

mpdesc_t *kmempool_del_mpdesc(kmempool_t *kmempool, uint_t relpnr, addr_t adr);
void mphead_init(mphead_t *initp) {
  initp->mlh_nr = 0;
  list_init(&initp->mlh_msalst);
}

void kmempool_init(kmempool_t *initp) {
  spinlock_init(&initp->mp_lock);
  list_init(&initp->mp_list);
  initp->mp_stus = 0;
  initp->mp_flgs = 0;
  spinlock_init(&initp->mp_pglock);
  spinlock_init(&initp->mp_oblock);
  initp->mp_pgmplnr = 0;
  initp->mp_obmplnr = 0;
  list_init(&initp->mp_pgmplmheadl);
  list_init(&initp->mp_obmplmheadl);
  initp->mp_pgmplmhcach = NULL;
  initp->mp_obmplmhcach = NULL;
#ifdef x86_64
  for (uint_t i = 0; i < PHYMSA_MAX; i++) {
    mphead_init(&initp->mp_msalsthead[i]);
  }
#endif
}

void kmempool_add_mpdesc(kmempool_t *kmplp, mpdesc_t *mpg, uint_t relpnr) {
  if (NULL == kmplp || NULL == mpg || 1 > relpnr) {
    panic("msadsc_add_kmempool fail\n");
  }
  cpuflg_t cpuflg;
  spinlock_cli(&kmplp->mp_lock, &cpuflg);
  if (relpnr >= (PHYMSA_MAX - 1)) {
    list_add(&mpg->mpd_list, &kmplp->mp_msalsthead[(PHYMSA_MAX - 1)].mlh_msalst);
    kmplp->mp_msalsthead[(PHYMSA_MAX - 1)].mlh_nr++;
  } else {
    list_add(&mpg->mpd_list, &kmplp->mp_msalsthead[relpnr].mlh_msalst);
    kmplp->mp_msalsthead[relpnr].mlh_nr++;
  }
  spinunlock_sti(&kmplp->mp_lock, &cpuflg);
}

mpdesc_t *kmempool_del_mpdesc(kmempool_t *kmplp, uint_t relpnr, addr_t adr) {
  if (NULL == kmplp || 1 > relpnr || NULL == adr)
    return NULL;
  cpuflg_t cpuflg;

  spinlock_cli(&kmplp->mp_lock, &cpuflg);

  mphead_t *mphead = NULL;
  if (relpnr >= (PHYMSA_MAX - 1)) {
    mphead = &kmplp->mp_msalsthead[(PHYMSA_MAX - 1)];
  } else {
    mphead = &kmplp->mp_msalsthead[relpnr];
  }

  list_t *tmplst = NULL;
  mpdesc_t *tmpmsa = NULL, *retmsa = NULL;
  list_for_each(tmplst, &mphead->mlh_msalst) {
    tmpmsa = list_entry(tmplst, mpdesc_t, mpd_list);
    if (mpdesc_ret_vaddr(tmpmsa) == adr) {
      list_del(&tmpmsa->mpd_list);
      retmsa = tmpmsa;
      mphead->mlh_nr--;
      break;
    }
  }

  spinunlock_sti(&kmplp->mp_lock, &cpuflg);
  return retmsa;
}

addr_t kmempool_pages_new(size_t msize) {
  kmempool_t *kmplp = &oskmempool;
  uint_t relpnr = 0;
  mpdesc_t *retmsa = NULL;
  retmsa = memory_divide_pages(&memgrob, msize >> 12, &relpnr, MA_TYPE_KRNL, DMF_RELDIV);
  if (NULL == retmsa) {
    return NULL;
  }
  kmempool_add_mpdesc(kmplp, retmsa, relpnr);
  return mpdesc_ret_vaddr(retmsa);
}

addr_t kmempool_obj_new(size_t msize) {
  return (addr_t)kmobj_alloc(msize);
}

addr_t kmempool_new(size_t mmsize) {
  if (mmsize > 2048)
    return kmempool_pages_new(mmsize);
  return kmempool_obj_new(mmsize);
}

addr_t krlnew(size_t mmsize) {
  if (mmsize == MALCSZ_MIN || mmsize > MALCSZ_MAX) {
    return NULL;
  }
  return kmempool_new(mmsize);
}

bool_t kmempool_pages_delete(addr_t adr, size_t sz) {
  kmempool_t *kmplp = &oskmempool;
  uint_t relpnr = sz >> PAGE_SZRBIT;
  mpdesc_t *retmsa = NULL;
  retmsa = kmempool_del_mpdesc(kmplp, relpnr, adr);
  if (NULL == retmsa) {
    return FALSE;
  }
  return memory_merge_pages(&memgrob, retmsa, relpnr);
}

bool_t kmempool_obj_delete(addr_t adr, size_t sz) {
  return kmobj_free((void *)adr, sz);
}

bool_t kmempool_delete(addr_t adr, size_t sz) {
  if (sz > 2048)
    return kmempool_pages_delete(adr, sz);
  return kmempool_obj_delete(adr, sz);
}

bool_t krldelete(addr_t adr, size_t sz) {
  if (adr == NULL || sz == MALCSZ_MIN || sz > MALCSZ_MAX)
    return FALSE;
  return kmempool_delete(adr, sz);
}
