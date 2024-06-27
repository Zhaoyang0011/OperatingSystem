#include <console.h>
#include <hal/memory/memgrob.h>
#include <memlayout.h>

HAL_DEFGLOB_VARIABLE(memgrob_t, memgrob);

void memmgrob_t_init(memgrob_t *initp) {
  list_init(&initp->mo_list);
  spinlock_init(&initp->mo_lock);
  initp->mo_stus = 0;
  initp->mo_flgs = 0;
  initp->mo_memsz = 0;
  initp->mo_maxpages = 0;
  initp->mo_freepages = 0;
  initp->mo_alocpages = 0;
  initp->mo_resvpages = 0;
  initp->mo_horizline = 0;
  initp->mo_pmagestat = NULL;
  initp->mo_pmagenr = 0;
  initp->mo_msadscstat = NULL;
  initp->mo_msanr = 0;
  initp->mo_mareastat = NULL;
  initp->mo_mareanr = 0;
  return;
}

void init_memgrob() {
  memmgrob_t_init(&memgrob);
  if (kernel_descriptor.mmap_adr == NULL || kernel_descriptor.mmap_nr == 0) {
	panic("Memory map structures error!\n");
  }
  if (kernel_descriptor.mp_desc_arr == NULL || kernel_descriptor.mp_desc_nr == 0) {
	panic("Memory page structures error!\n");
  }
  if (kernel_descriptor.ma_desc_arr == NULL || kernel_descriptor.ma_nr == 0) {
	panic("Memory area structures error!\n");
  }

  memgrob.mo_pmagestat = (physical_memory_t *)P2V(kernel_descriptor.mmap_adr);
  memgrob.mo_pmagenr = kernel_descriptor.mmap_nr;
  memgrob.mo_msadscstat = (mpdesc_t *)P2V(kernel_descriptor.mp_desc_arr);
  memgrob.mo_msanr = kernel_descriptor.mp_desc_nr;
  memgrob.mo_mareastat = (memarea_t *)P2V(kernel_descriptor.ma_desc_arr);
  memgrob.mo_mareanr = kernel_descriptor.ma_nr;
  memgrob.mo_memsz = kernel_descriptor.mp_desc_nr << PAGE_SIZE;
  memgrob.mo_maxpages = kernel_descriptor.mp_desc_nr;
  uint_t aidx = 0;
  for (uint_t i = 0; i < memgrob.mo_msanr; i++) {
	mpflgs_t flags = memgrob.mo_msadscstat[i].mpd_indxflgs;
	if (1 == flags.mpf_uindx && MF_MOCTY_KRNL == flags.mpf_mocty &&
		PAF_ALLOC == memgrob.mo_msadscstat[i].mpd_adrflgs.paf_alloc) {
	  aidx++;
	}
  }
  memgrob.mo_alocpages = aidx;
  memgrob.mo_freepages = memgrob.mo_maxpages - memgrob.mo_alocpages;
  return;
}