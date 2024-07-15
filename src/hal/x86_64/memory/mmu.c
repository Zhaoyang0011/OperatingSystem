#include <hal/memory/mmu.h>
#include <hal/memory/mempage.h>
#include <hal/memory/memarea.h>
#include <hal/memory/memdivmer.h>
#include <type.h>
#include <string.h>
#include <spinlock.h>
#include <memlayout.h>
#include "../io.h"

KLINE uint_t mmu_tdire_index(addr_t vadrs) {
  return (uint_t)((vadrs >> TDIRE_IV_RSHTBIT) & TDIRE_IV_BITMASK);
}

KLINE uint_t mmu_sdire_index(addr_t vadrs) {
  return (uint_t)((vadrs >> SDIRE_IV_RSHTBIT) & SDIRE_IV_BITMASK);
}

KLINE uint_t mmu_idire_index(addr_t vadrs) {
  return (uint_t)((vadrs >> IDIRE_IV_RSHTBIT) & IDIRE_IV_BITMASK);
}

KLINE uint_t mmu_mdire_index(addr_t vadrs) {
  return (uint_t)((vadrs >> MDIRE_IV_RSHTBIT) & MDIRE_IV_BITMASK);
}

void mmudesc_init(mmudesc_t *initp) {
  if (NULL == initp) {
    return;
  }

  spinlock_init(&initp->mud_lock);
  initp->mud_stus = 0;
  initp->mud_flag = 0;
  initp->mud_tdirearr = NULL;
  initp->mud_cr3.c3s_entry = 0;
  list_init(&initp->mud_tdirhead);
  list_init(&initp->mud_sdirhead);
  list_init(&initp->mud_idirhead);
  list_init(&initp->mud_mdirhead);
  initp->mud_tdirmsanr = 0;
  initp->mud_sdirmsanr = 0;
  initp->mud_idirmsanr = 0;
  initp->mud_mdirmsanr = 0;
}

KLINE void tdirearr_t_init(tdirearr_t *init) {
  if (NULL == init) {
    return;
  }
  memset((void *)init, 0, sizeof(tdirearr_t));
  return;
}

mpdesc_t *mmu_new_tdirearr(mmudesc_t *mmulocked) {
  tdirearr_t *tdirearr = NULL;
  uint64_t pages = 1, retpnr = 0;
  mpdesc_t *msa = NULL;
  if (NULL == mmulocked) {
    return NULL;
  }

  msa = memory_divide_pages(&memgrob, pages, &retpnr, MA_TYPE_KRNL, DMF_RELDIV);
  if (NULL == msa) {
    return NULL;
  }

  tdirearr = (tdirearr_t *)mpdesc_ret_vaddr(msa);

  tdirearr_t_init(tdirearr);

  list_add(&msa->mpd_list, &mmulocked->mud_tdirhead);
  mmulocked->mud_tdirmsanr++;
  mmulocked->mud_tdirearr = tdirearr;

  return msa;
}

bool_t hal_mmu_init(mmudesc_t *mmu) {
  bool_t rets = FALSE;
  addr_t vcr3 = NULL;

  if (NULL == mmu) {
    return FALSE;
  }

  spinlock_init(&mmu->mud_lock);

  if (mmu_new_tdirearr(mmu) == NULL) {
    rets = FALSE;
    goto out;
  }

//  vcr3 = P2V(kernel_desc.mb_pml4padr);

  memcpy((void *)vcr3, (void *)mmu->mud_tdirearr, sizeof(tdirearr_t));

  mmu->mud_cr3.c3s_entry = (uint64_t)V2P((addr_t)mmu->mud_tdirearr);
  mmu->mud_tdirearr->tde_arr[0].t_entry = 0;
  rets = TRUE;

out:
  spin_unlock(&mmu->mud_lock);
  return rets;
}

void hal_mmu_load(mmudesc_t *mmu) {
  if (NULL == mmu) {
    return;
  }

  spin_lock(&mmu->mud_lock);
  if (NULL == mmu->mud_tdirearr || 0 != (((uint64_t)(mmu->mud_tdirearr)) & 0xfff)
      || (unsigned long long)mmu->mud_tdirearr < KRNL_VIRTUAL_ADDRESS_START) {
    goto out;
  }

  mmu->mud_cr3.c3s_entry = V2P((addr_t)mmu->mud_tdirearr);
  write_cr3((uint_t)(mmu->mud_cr3.c3s_entry));

out:
  spin_unlock(&mmu->mud_lock);
}