#ifndef _MMU_H
#define _MMU_H

#include <type.h>
#include <spinlock.h>
#include <struct/list.h>

#ifdef x86_64

#define TDIRE_MAX (512)
#define SDIRE_MAX (512)
#define IDIRE_MAX (512)
#define MDIRE_MAX (512)

#define MDIRE_IV_RSHTBIT (12)
#define MDIRE_IV_BITMASK (0x1ffUL)
#define MSA_PADR_LSHTBIT (12)
#define MDIRE_PADR_LSHTBIT (12)
#define IDIRE_IV_RSHTBIT (21)
#define IDIRE_IV_BITMASK (0x1ffUL)
#define IDIRE_PADR_LSHTBIT (12)
#define SDIRE_IV_RSHTBIT (30)
#define SDIRE_IV_BITMASK (0x1ffUL)
#define SDIRE_PADR_LSHTBIT (12)
#define TDIRE_IV_RSHTBIT (39)
#define TDIRE_IV_BITMASK (0x1ffUL)

typedef struct TDIREFLAGS {
  uint64_t t_p: 1;     //0
  uint64_t t_rw: 1;    //1
  uint64_t t_us: 1;    //2
  uint64_t t_pwt: 1;   //3
  uint64_t t_pcd: 1;   //4
  uint64_t t_a: 1;     //5
  uint64_t t_ig1: 1;   //6
  uint64_t t_rv1: 1;   //7
  uint64_t t_ig2: 4;   //8\9\10\11
  uint64_t t_sdir: 40; //12
  uint64_t t_ig3: 11;  //52
  uint64_t t_xd: 1;    //63
} __attribute__((packed)) tdireflags_t;

typedef struct TDIRE {
  union {
    tdireflags_t t_flags;
    uint64_t t_entry;
  } __attribute__((packed));

} __attribute__((packed)) tdire_t;

typedef struct TDIREARR {
  tdire_t tde_arr[TDIRE_MAX];
} __attribute__((packed)) tdirearr_t;

typedef struct c3s_flags {
  uint64_t c3s_pcid: 12;  //0
  uint64_t c3s_plm4a: 40; //12
  uint64_t c3s_rv: 11;    //52
  uint64_t c3s_tbc: 1;    //63
} __attribute__((packed)) cr3sflgs_t;

typedef struct CR3S {
  union {
    cr3sflgs_t c3s_c3sflgs;
    uint64_t c3s_entry;
  };
} __attribute__((packed)) cr3s_t;

typedef struct mmu_descriptor {
  spinlock_t mud_lock;
  uint64_t mud_stus;
  uint64_t mud_flag;
  tdirearr_t *mud_tdirearr;
  cr3s_t mud_cr3;
  list_t mud_tdirhead;
  list_t mud_sdirhead;
  list_t mud_idirhead;
  list_t mud_mdirhead;
  uint_t mud_tdirmsanr;
  uint_t mud_sdirmsanr;
  uint_t mud_idirmsanr;
  uint_t mud_mdirmsanr;
} mmudesc_t;

void mmudesc_init(mmudesc_t *);
void hal_mmu_load(mmudesc_t *mmu);

#endif

#endif