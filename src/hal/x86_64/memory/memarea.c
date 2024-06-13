#include "../io.h"
#include "type.h"
#include <console.h>
#include <hal/halglobal.h>
#include <hal/memory/memarea.h>

void mpaflist_t_init(mpaflist_t *mapflist, uint32_t stus, uint_t oder, uint_t oderpnr) {
    spinlock_init(&mapflist->af_lock);
    mapflist->af_stus = stus;
    mapflist->af_oder = oder;
    mapflist->af_oderpnr = oderpnr;
    mapflist->af_fobjnr = 0;
    mapflist->af_mobjnr = 0;
    mapflist->af_alcindx = 0;
    mapflist->af_freindx = 0;
    list_init(&mapflist->af_frelist);
    list_init(&mapflist->af_alclist);
}

void memdivmer_t_init(memdivmer_t *memdivmer) {
    spinlock_init(&memdivmer->dm_lock);
    memdivmer->dm_stus = 0;
    memdivmer->dm_dmmaxindx = 0;
    memdivmer->dm_phydmindx = 0;
    memdivmer->dm_predmindx = 0;
    memdivmer->dm_divnr = 0;
    memdivmer->dm_mernr = 0;

    for (uint_t li = 0; li < MDIVMER_ARR_LMAX; li++) {
        mpaflist_t_init(&memdivmer->dm_mdmlst[li], MPAF_STUS_DIVM, li, (1UL << li));
    }
    mpaflist_t_init(&memdivmer->dm_onelst, MPAF_STUS_ONEM, 0, 1UL);
    return;
}

void memarea_t_init(memarea_t *memarea) {
    list_init(&memarea->ma_list);
    spinlock_init(&memarea->ma_lock);
    memarea->ma_stus = 0;
    memarea->ma_flgs = 0;
    memarea->ma_type = MA_TYPE_INIT;
    memarea->ma_maxpages = 0;
    memarea->ma_allocpages = 0;
    memarea->ma_freepages = 0;
    memarea->ma_resvpages = 0;
    memarea->ma_horizline = 0;
    memarea->ma_logicstart = 0;
    memarea->ma_logicend = 0;
    memarea->ma_logicsz = 0;
    memarea->ma_effectstart = 0;
    memarea->ma_effectend = 0;
    memarea->ma_effectsz = 0;
    list_init(&memarea->ma_allmsadsclst);
    memarea->ma_allmsadscnr = 0;
    memdivmer_t_init(&memarea->ma_mdmdata);
    memarea->ma_privp = NULL;
}

void init_memory_area() {
    for (uint_t i = 0; i < MEMAREA_MAX; i++) {
        memarea_t_init(&memarea_arr[i]);
    }

    memarea_arr[0].ma_type = MA_TYPE_HWAD;
    memarea_arr[0].ma_logicstart = MA_HWAD_LSTART;
    memarea_arr[0].ma_logicend = MA_HWAD_LEND;
    memarea_arr[0].ma_logicsz = MA_HWAD_LSZ;

    memarea_arr[1].ma_type = MA_TYPE_KRNL;
    memarea_arr[1].ma_logicstart = MA_KRNL_LSTART;
    memarea_arr[1].ma_logicend = MA_KRNL_LEND;
    memarea_arr[1].ma_logicsz = MA_KRNL_LSZ;

    memarea_arr[2].ma_type = MA_TYPE_PROC;
    memarea_arr[2].ma_logicstart = MA_PROC_LSTART;
    memarea_arr[2].ma_logicend = MA_PROC_LEND;
    memarea_arr[2].ma_logicsz = MA_PROC_LSZ;

    memarea_arr[3].ma_type = MA_TYPE_SHAR;

    kernel_descriptor.ma_desc_arr = (uint64_t) memarea_arr;
    kernel_descriptor.ma_nr = MEMAREA_MAX;
    kernel_descriptor.ma_sz = sizeof(memarea_t) * MEMAREA_MAX;
}

void
search_memarea_mempage(const memarea_t *memarea, const mpdesc_t *mpdesc_arr, uint64_t mpnr, int *sindx, int *eindx) {
    *sindx = -1;
    *eindx = -1;

    uint64_t saddr = memarea->ma_logicstart;
    uint64_t eaddr = memarea->ma_logicend;
    saddr = saddr & PAF_ADDR_MASK;

    for (int i = 0; i < mpnr; ++i) {
        uint64_t page_addr = (mpdesc_arr[i].mpd_addr & PAF_ADDR_MASK);
        if (page_addr >= saddr && page_addr < eaddr) {
            *sindx = i;
            break;
        }
    }

    if (*sindx < 0)
        return;

    for (int i = *sindx; i < mpnr; ++i) {
        if ((mpdesc_arr[i].mpd_addr & PAF_ADDR_MASK) > eaddr) {
            *eindx = i - 1;
            break;
        }
    }

    if (*eindx < 0)
        *eindx = mpnr - 1;
}

int set_mempage_memarea_one(mpdesc_t *mpdesc_arr, uint64_t mpnr, const memarea_t *memarea) {
    if (mpdesc_arr == NULL || memarea == NULL || mpnr == 0)
        return -1;

    int sindx;
    int eindx;

    search_memarea_mempage(memarea, mpdesc_arr, mpnr, &sindx, &eindx);

    if (sindx < 0 || eindx < 0)
        return 0;

    int retnr = 0;
    for (int i = sindx; i <= eindx; ++i) {
        if (mpdesc_arr[i].mpd_indxflgs.mpf_marty == MF_MARTY_INIT) {
            mpdesc_arr[i].mpd_indxflgs.mpf_marty = memarea->ma_type;
            retnr++;
        }
    }

    return retnr;
}

bool_t set_mempage_memarea() {
    mpdesc_t *mpdesc_arr = (mpdesc_t *) kernel_descriptor.mp_desc_arr;
    uint64_t mpnr = kernel_descriptor.mp_desc_nr;

    // set memory pages flags belong to hardware memory area
    int hanr = set_mempage_memarea_one(mpdesc_arr, mpnr, &memarea_arr[0]);
    if (hanr < 0)
        return FALSE;

    // set memory pages belong to kernel memory area
    int krlnr = set_mempage_memarea_one(mpdesc_arr, mpnr, &memarea_arr[1]);
    if (krlnr < 0)
        return FALSE;

    // set memory pages belong to user memory area
    int procnr = set_mempage_memarea_one(mpdesc_arr, mpnr, &memarea_arr[2]);
    if (procnr < 0)
        return FALSE;

    if (hanr + krlnr + procnr != mpnr)
        return FALSE;

    return TRUE;
}

bool_t scan_continuous_free_memorypage(mpdesc_t *scan_start, mpdesc_t *scan_end, const memarea_t *memarea,
                                       mpdesc_t **ret_start, mpdesc_t **ret_end) {
    if (scan_start == NULL || scan_end == NULL || memarea == NULL || ret_start == NULL || ret_end == NULL)
        return FALSE;

    mpdesc_t *continous_start = scan_start;
    for (; continous_start <= scan_end; ++continous_start) {
        if (continous_start->mpd_adrflgs.paf_alloc == PAF_NO_ALLOC &&
            continous_start->mpd_indxflgs.mpf_marty == memarea->ma_type)
            break;
    }

    *ret_start = NULL;
    *ret_end = NULL;
    if (continous_start->mpd_adrflgs.paf_alloc != PAF_NO_ALLOC ||
        continous_start->mpd_indxflgs.mpf_marty != memarea->ma_type) {
        return TRUE;
    }

    *ret_start = continous_start;
    for (int i = 1; continous_start + i <= scan_end; ++i) {
        uint64_t paddr = continous_start[i - 1].mpd_addr & PAF_ADDR_MASK;
        uint64_t caddr = continous_start[i].mpd_addr & PAF_ADDR_MASK;

        if (paddr + PAGE_SIZE != caddr || continous_start[i].mpd_adrflgs.paf_alloc != PAF_NO_ALLOC ||
            continous_start[i].mpd_indxflgs.mpf_marty != memarea->ma_type ||
            continous_start[i].mpd_indxflgs.mpf_marty != memarea->ma_type) {
            *ret_end = &continous_start[i - 1];
            return TRUE;
        }
    }

    *ret_end = scan_end;
    return TRUE;
}

bool_t load_continous_mempage_mpaflist(mpaflist_t *mpaflst, mpdesc_t *start, mpdesc_t *end) {
    if (end - start + 1 != mpaflst->af_oderpnr)
        return FALSE;
    list_add(&start->mpd_list, &mpaflst->af_frelist);

    start->mpd_odlink = end;
    start->mpd_indxflgs.mpf_olkty = MF_OLKTY_ODER;

    end->mpd_odlink = mpaflst;
    end->mpd_indxflgs.mpf_olkty = MF_OLKTY_BAFH;

    mpaflst->af_fobjnr++;
    mpaflst->af_mobjnr++;
    return TRUE;
}

bool_t load_continous_mempage_memarea(memarea_t *memarea, mpdesc_t *continous_start, mpdesc_t *continous_end) {
    if (continous_start == NULL || continous_end == NULL || continous_start > continous_end)
        return FALSE;

    uint64_t start = continous_start->mpd_addr & PAF_ADDR_MASK;
    uint64_t end = continous_end->mpd_addr & PAF_ADDR_MASK;
    if (continous_end - continous_start != (end - start) >> 12)
        return FALSE;

    uint64_t n = continous_end - continous_start + 1;
    mpdesc_t *load_start = continous_start;
    while (n > 0) {
        sint32_t shar = search_64lbits(n);

        uint32_t load_num = 1 << shar;
        if (!load_continous_mempage_mpaflist(&memarea->ma_mdmdata.dm_mdmlst[shar], load_start,
                                             load_start + load_num - 1))
            return FALSE;

        memarea->ma_maxpages += load_num;
        memarea->ma_freepages += load_num;
        memarea->ma_allmsadscnr += load_num;

        load_start += load_num;
        n -= load_num;
    }

    return TRUE;
}

bool_t load_mempage_memarea_one(memarea_t *memarea, mpdesc_t *mpdesc_arr, uint64_t mpnr) {
    if (mpdesc_arr == NULL || memarea == NULL || mpnr == 0)
        return FALSE;

    int sindx;
    int eindx;

    search_memarea_mempage(memarea, mpdesc_arr, mpnr, &sindx, &eindx);
    if (sindx < 0 || eindx < 0)
        return TRUE;

    for (mpdesc_t *scan_start = mpdesc_arr + sindx; scan_start <= mpdesc_arr + eindx;) {
        mpdesc_t *continous_start = NULL;
        mpdesc_t *continous_end = NULL;
        bool_t retb =
                scan_continuous_free_memorypage(scan_start, &mpdesc_arr[eindx], memarea, &continous_start,
                                                &continous_end);

        if (retb == FALSE)
            panic("Scan continous free memory page error!");
        if (continous_start == NULL || continous_end == NULL)
            break;

        if (!load_continous_mempage_memarea(memarea, continous_start, continous_end))
            panic("Load memory page error");

        scan_start = continous_end + 1;
    }

    return TRUE;
}

bool_t load_mempage_memarea_core() {
    mpdesc_t *mpdesc_arr = (mpdesc_t *) kernel_descriptor.mp_desc_arr;
    uint64_t mpnr = kernel_descriptor.mp_desc_nr;

    for (int i = 0; i < MEMAREA_MAX - 1; ++i) {
        if (!load_mempage_memarea_one(&memarea_arr[i], mpdesc_arr, mpnr))
            return FALSE;
    }

    return TRUE;
}

// load memory page to memory area
void load_mempage_memarea() {
    if (set_mempage_memarea() == FALSE)
        panic("Set memory page area error!");
    if (load_mempage_memarea_core() == FALSE)
        panic("load memory page to memory area error!");
    return;
}
