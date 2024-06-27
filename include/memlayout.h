#ifndef _MEMLAYOUT_H
#define _MEMLAYOUT_H

/**
 * kernel information macro and structure
 */

// virtual memory page macro
#define KRNL_VIRTUAL_ADDRESS_START 0xffff800000000000
#define P2V(m) (KRNL_VIRTUAL_ADDRESS_START + m)
#define KPML4_P (1 << 0)
#define KPML4_RW (1 << 1)
#define KPML4_US (1 << 2)
#define KPML4_PWT (1 << 3)
#define KPML4_PCD (1 << 4)
#define KPML4_A (1 << 5)

#define KPDPTE_P (1 << 0)
#define KPDPTE_RW (1 << 1)
#define KPDPTE_US (1 << 2)
#define KPDPTE_PWT (1 << 3)
#define KPDPTE_PCD (1 << 4)
#define KPDPTE_A (1 << 5)

#define KPDE_P (1 << 0)
#define KPDE_RW (1 << 1)
#define KPDE_US (1 << 2)
#define KPDE_PWT (1 << 3)
#define KPDE_PCD (1 << 4)
#define KPDE_A (1 << 5)
#define KPDE_D (1 << 6)
#define KPDE_PS (1 << 7)
#define KPDE_G (1 << 8)
#define KPDE_PAT (1 << 12)

#define KPML4_SHIFT 39
#define KPDPTTE_SHIFT 30
#define KPDP_SHIFT 21
#define PGENTY_SIZE 512

#endif