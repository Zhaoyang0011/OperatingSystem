#ifndef _KRLMEMPOOL_H
#define _KRLMEMPOOL_H

#include <spinlock.h>
#include <struct/list.h>
#include <kernel/krlglobal.h>

#define MPALCLST_MAX 5

#define PMPLMAP_MAX 32
#define KMEMPALCSZ_MIN 1
#define KMEMPALCSZ_MAX 0x400000

#define OBJSORPAGE 2048
#define KPMPORHALM (PAGE_SIZE*31)

#define MPLHTY_PAGE 1
#define MPLHTY_OBJS 2

#define OBJS_ALIGN(x) ALIGN(x,4)
#define PHYMSA_MAX 512
#define PAGE_SIZE 0x1000
#define PAGE_SZRBIT (12)

#define MALCSZ_MIN (0)
#define MALCSZ_MAX (0x400000)

typedef struct s_PGLMAP {
  addr_t pgl_start;
  struct s_PGLMAP *pgl_next;
} pglmap_t;

typedef struct memory_page_list_head {
  spinlock_t mh_lock;
  list_t mh_list;
  uint_t mh_hedty;
  addr_t mh_start;
  addr_t mh_end;
  addr_t mh_firtfreadr;
  pglmap_t *mh_firtpmap;
  uint_t mh_objnr;
  uint_t mh_aliobsz;
  uint_t mh_objsz;
  uint_t mh_nxtpsz;
  uint_t mh_afindx;
  uint_t mh_pmnr;
  pglmap_t *mh_pmap;
} mplhead_t;

typedef struct memory_page_head {
  uint_t mlh_nr;
  list_t mlh_msalst;
} mphead_t;

typedef struct kernel_memory_pool {
  spinlock_t mp_lock;
  list_t mp_list;
  uint_t mp_stus;
  uint_t mp_flgs;
  spinlock_t mp_pglock;
  spinlock_t mp_oblock;
  uint_t mp_pgmplnr;
  uint_t mp_obmplnr;
  list_t mp_pgmplmheadl;
  list_t mp_obmplmheadl;
  mplhead_t *mp_pgmplmhcach;
  mplhead_t *mp_obmplmhcach;
#ifdef x86_64
  mphead_t mp_msalsthead[PHYMSA_MAX];
#endif
} kmempool_t;

void kmempool_init(kmempool_t *);
addr_t krlnew(size_t);
bool_t krldelete(addr_t,size_t);

extern KRL_DEFGLOB_VARIABLE(kmempool_t, oskmempool);

#endif
