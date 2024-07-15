#include <hal/memory/mmu.h>
#include <type.h>
#include <spinlock.h>
#include <memlayout.h>
#include "../io.h"

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