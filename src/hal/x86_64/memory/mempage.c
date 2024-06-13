#include "hal/memory/phymem.h"
#include <console.h>
#include <hal/halglobal.h>
#include <hal/memory/mempage.h>
#include <ldrtype.h>

bool_t ret_mpdesc_addrandsz(kernel_desc_t *kernel, mpdesc_t **retmpsadr, uint64_t *retmpnr) {
    if (kernel == NULL || retmpsadr == NULL || retmpnr == NULL)
        return FALSE;

    if (kernel->mmap_nr < 1 || NULL == kernel->mmap_adr ||
        (kernel->mmap_nr * sizeof(physical_memory_t)) != kernel->mmap_sz) {
        *retmpsadr = NULL;
        *retmpnr = 0;
        return FALSE;
    }

    uint64_t memory_size = 0, memory_page_num = 0;
    physical_memory_t *pmarr = (physical_memory_t *) kernel->mmap_adr;
    for (uint64_t i = 0; i < kernel->mmap_nr; i++) {
        if (pmarr[i].pm_type == PM_T_OSAPUSERRAM) {
            memory_size += pmarr[i].pm_size;
            memory_page_num += (pmarr[i].pm_size >> 12);
        }
    }

    if (memory_size == 0 || (memory_size >> 1) < 1 || memory_page_num < 1) {
        *retmpsadr = NULL;
        *retmpnr = 0;
        return FALSE;
    }

    *retmpsadr = (mpdesc_t *) kernel->next_pg;
    *retmpnr = memory_page_num;

    return TRUE;
}

void init_mpdsc(mpdesc_t *mp) {
    list_init(&mp->mpd_list);
    spinlock_init(&mp->mpd_lock);
    mp->mpd_indxflgs.mpf_olkty = MF_OLKTY_INIT;
    mp->mpd_indxflgs.mpf_lstty = MF_LSTTY_LIST;
    mp->mpd_indxflgs.mpf_mocty = MF_MOCTY_FREE;
    mp->mpd_indxflgs.mpf_marty = MF_MARTY_INIT;
    mp->mpd_indxflgs.mpf_uindx = MF_UINDX_INIT;

    mp->mpd_adrflgs.paf_alloc = PAF_NO_ALLOC;
    mp->mpd_adrflgs.paf_shared = PAF_NO_SHARED;
    mp->mpd_adrflgs.paf_swap = PAF_NO_SWAP;
    mp->mpd_adrflgs.paf_cache = PAF_NO_CACHE;
    mp->mpd_adrflgs.paf_kmap = PAF_NO_KMAP;
    mp->mpd_adrflgs.paf_lock = PAF_NO_LOCK;
    mp->mpd_adrflgs.paf_dirty = PAF_NO_DIRTY;
    mp->mpd_adrflgs.paf_busy = PAF_NO_BUSY;
    mp->mpd_adrflgs.paf_rsv = PAF_RSV_VAL;
    mp->mpd_adrflgs.paf_paddr = PAF_INIT_PADRS;
    mp->mpd_odlink = NULL;
}

void write_one_mpdsc(mpdesc_t *mp, uint64_t phyadr) {
    init_mpdsc(mp);
    phyadrflgs_t *tmp = (phyadrflgs_t *) (&phyadr);
    mp->mpd_adrflgs.paf_paddr = tmp->paf_paddr;
}

bool_t check_mpdesc_order(mpdesc_t *mpdesc_arr, uint64_t mpnr) {
    for (int i = 1; i < mpnr; ++i) {
        if (mpdesc_arr[i].mpd_addr < mpdesc_arr[i - 1].mpd_addr)
            return FALSE;
    }
    return TRUE;
}

uint64_t init_mpdsc_core(kernel_desc_t *kernel, mpdesc_t *mpsadr, uint64_t mpnr) {
    if (kernel == NULL || mpsadr == NULL || mpnr < 1)
        return 0;

    physical_memory_t *pmarr = (physical_memory_t *) kernel->mmap_adr;

    uint64_t retmpnr = 0;
    for (uint64_t i = 0; i < kernel->mmap_nr; i++) {
        if (PM_T_OSAPUSERRAM != pmarr[i].pm_type)
            continue;

        for (uint64_t start = pmarr[i].pm_start; start < pmarr[i].pm_end; start += 4096) {
            if ((start + 4096 - 1) <= pmarr[i].pm_end) {
                write_one_mpdsc(&mpsadr[retmpnr], start);
                retmpnr++;
            }
        }
    }

    if (!check_mpdesc_order(mpsadr, mpnr)) {
        panic("memory page order error!");
    }

    return retmpnr;
}

void init_memory_page() {
    uint64_t corempnr = 0, mpnr = 0;
    mpdesc_t *mpsadr = NULL;
    kernel_desc_t *kernel = &kernel_descriptor;
    if (ret_mpdesc_addrandsz(kernel, &mpsadr, &mpnr) == FALSE) {
        panic("init_msadsc ret_msadsc_vadrandsz err\n");
    }

    corempnr = init_mpdsc_core(kernel, mpsadr, mpnr);
    if (corempnr != mpnr) {
        panic("init_msadsc_core err\n");
    }

    kernel_descriptor.mp_desc_arr = (uint64_t) mpsadr;
    kernel_descriptor.mp_desc_nr = corempnr;
    kernel_descriptor.mp_desc_sz = corempnr * sizeof(mpdesc_t);

    kernel_descriptor.next_pg = P4K_ALIGN(kernel_descriptor.mp_desc_arr + kernel_descriptor.mp_desc_sz);
    kernel_descriptor.kernel_size = kernel_descriptor.next_pg - kernel_descriptor.kernel_start;
}

mpdesc_t *search_mempage(const mpdesc_t *mpdesc_arr, uint64_t mpnr, uint64_t saddr) {
    saddr = saddr & PAF_ADDR_MASK;
    int i = saddr >> 12;
    if (i >= mpnr)
        return NULL;

    uint64_t mp_addr = (mpdesc_arr[i].mpd_addr & PAF_ADDR_MASK);
    if (mp_addr > saddr) {
        for (; i > 0; --i) {
            if ((mpdesc_arr[i].mpd_addr & PAF_ADDR_MASK) == saddr)
                return mpdesc_arr + i;
        }
        return NULL;
    }

    for (; i < mpnr; ++i) {
        if ((mpdesc_arr[i].mpd_addr & PAF_ADDR_MASK) == saddr)
            return mpdesc_arr + i;
    }
    return NULL;
}

// 搜索一段内存地址空间所对应的mpgdesc_t结构
uint64_t search_segment_occupympg(mpdesc_t *mpdesc_arr, uint64_t mpnr, uint64_t saddr, uint64_t eaddr) {
    mpdesc_t *mp_start = search_mempage(mpdesc_arr, mpnr, saddr);
    if (mp_start == NULL) {
        return 0;
    }

    uint64_t num = ((eaddr - saddr) >> 12) + 1;

    if (mpnr - num < mp_start - mpdesc_arr) {
        return 0;
    }

    if ((mp_start[num - 1].mpd_addr & PAF_ADDR_MASK) != (eaddr & PAF_ADDR_MASK)) {
        return 0;
    }

    uint64_t mphyadr = 0, occ_mpnr = 0;
    for (uint64_t tmpadr = saddr; tmpadr < eaddr; tmpadr += PAGE_SIZE, occ_mpnr++) {
        // 从开始地址对应的第一个msadsc_t结构开始设置，直到结束地址对应的最后一个masdsc_t结构
        mphyadr = mp_start[occ_mpnr].mpd_addr & PAF_ADDR_MASK;
        if (mphyadr != tmpadr) {
            return 0;
        }
        if (MF_MOCTY_FREE != mp_start[occ_mpnr].mpd_indxflgs.mpf_mocty ||
            0 != mp_start[occ_mpnr].mpd_indxflgs.mpf_uindx ||
            PAF_NO_ALLOC != mp_start[occ_mpnr].mpd_adrflgs.paf_alloc) {
            return 0;
        }
        // 设置msadsc_t结构为已经分配，已经分配给内核
        mp_start[occ_mpnr].mpd_indxflgs.mpf_mocty = MF_MOCTY_KRNL;
        mp_start[occ_mpnr].mpd_indxflgs.mpf_uindx++;
        mp_start[occ_mpnr].mpd_adrflgs.paf_alloc = PAF_ALLOC;
    }

    if (occ_mpnr != num)
        return 0;

    return occ_mpnr;
}

bool_t search_occupympgdesc_core() {
    mpdesc_t *mp_arr = (mpdesc_t *) kernel_descriptor.mp_desc_arr;
    uint64_t msanr = kernel_descriptor.mp_desc_nr;
    // 搜索BIOS中断表占用的内存页所对应msadsc_t结构
    uint64_t bios_occu = search_segment_occupympg(mp_arr, msanr, 0, 0x1000 - 1);
    if (0 == bios_occu) {
        return FALSE;
    }
    // 搜索内核栈占用的内存页所对应msadsc_t结构
    uint64_t krlstack_occu =
            search_segment_occupympg(mp_arr, msanr, kernel_descriptor.stack_init_adr,
                                     kernel_descriptor.stack_init_adr + kernel_descriptor.stack_size - 1);
    if (0 == krlstack_occu) {
        return FALSE;
    }
    // 搜索内核占用的内存页所对应msadsc_t结构
    uint64_t krlpage_occu =
            search_segment_occupympg(mp_arr, msanr, KINITPAGE_PHYADR, KINITPAGE_PHYADR + 0x1000 * 19 - 1);
    if (0 == krlpage_occu) {
        return FALSE;
    }
    uint64_t krl_occu =
            search_segment_occupympg(mp_arr, msanr, kernel_descriptor.kernel_start, kernel_descriptor.next_pg - 1);
    if (0 == krl_occu) {
        return FALSE;
    }

    return TRUE;
}

void init_mempage_occupation() {
    // 实际初始化搜索内核占用的内存页面
    if (search_occupympgdesc_core() == FALSE) {
        panic("search_occupympgdsc_core fail\n");
    }
    return;
}