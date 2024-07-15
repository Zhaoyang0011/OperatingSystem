#include <kernel/virmem.h>
#include <kernel/krlmempool.h>
#include <kernel/krlglobal.h>
#include <kernel/thread/scheduler.h>

// virmem.c
KRL_DEFGLOB_VARIABLE(kvmemcboxmgr_t, kvmcbox_manager);
KRL_DEFGLOB_VARIABLE(pvmspacdesc_t, initial_pvmspacedesc);

// krlmempool.h
KRL_DEFGLOB_VARIABLE(kmempool_t, oskmempool);

// scheduler.h
KRL_DEFGLOB_VARIABLE(schedclass_t, scheduler);