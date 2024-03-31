#include "memarea.h"
#include <hal/halglobal.h>

void mpaflist_t_init(mpaflist_t *mapflist, uint32_t stus, uint_t oder, uint_t oderpnr)
{
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

void memdivmer_t_init(memdivmer_t *memdivmer)
{
    spinlock_init(&memdivmer->dm_lock);
    memdivmer->dm_stus = 0;
    memdivmer->dm_dmmaxindx = 0;
    memdivmer->dm_phydmindx = 0;
    memdivmer->dm_predmindx = 0;
    memdivmer->dm_divnr = 0;
    memdivmer->dm_mernr = 0;

    for (uint_t li = 0; li < MDIVMER_ARR_LMAX; li++)
    {
        mpaflist_t_init(&memdivmer->dm_mdmlielst[li], MPAF_STUS_DIVM, li, (1UL << li));
    }
    mpaflist_t_init(&memdivmer->dm_onemsalst, MPAF_STUS_ONEM, 0, 1UL);
    return;
}

void memarea_t_init(memarea_t *memarea)
{
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

void init_memory_area()
{
    for (uint_t i = 0; i < MEMAREA_MAX; i++)
    {
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

    kernel_descriptor.ma_desc_arr = (uint64_t)memarea_arr;
    kernel_descriptor.ma_nr = MEMAREA_MAX;
    kernel_descriptor.ma_sz = sizeof(memarea_t) * MEMAREA_MAX;
}