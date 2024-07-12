#ifndef _MEMGROB_H
#define _MEMGROB_H

#include <hal/memory/memarea.h>
#include <hal/memory/memobject.h>
#include <hal/memory/mempage.h>
#include <hal/memory/phymem.h>
#include <spinlock.h>
#include <struct/list.h>
#include <semaphore.h>

typedef struct memgrob {
  list_t mo_list;
  spinlock_t mo_lock;
  uint_t mo_stus;
  uint_t mo_flgs;
  sem_t mo_sem;
  uint64_t mo_memsz;
  uint64_t mo_maxpages;
  uint64_t mo_freepages;
  uint64_t mo_alocpages;
  uint64_t mo_resvpages;
  uint64_t mo_horizline;
  physical_memory_t *mo_pmagestat;
  uint64_t mo_pmagenr;
  mpdesc_t *mo_msadscstat;
  uint64_t mo_msanr;
  memarea_t *mo_mareastat;
  uint64_t mo_mareanr;
  kmomgrlist_header_t mo_mobmgr;
  // void *mo_privp;
  // void *mo_extp;
} memgrob_t;

void init_memgrob();

HAL_DEFGLOB_VARIABLE(memgrob_t, memgrob);

#endif