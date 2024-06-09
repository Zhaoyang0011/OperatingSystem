#include <hal/memory/memgrob.h>
#include <hal/memory/memobject.h>

void mo_mgr_list_init(momgrlist_t *initp, size_t mobsz)
{
    list_init(&initp->ol_emplst);
    initp->ol_cache = NULL;
    initp->ol_emnr = 0;
    initp->ol_sz = mobsz;
}

void init_memory_object_manager()
{
    momgrheader_t *initp = &memgrob.mo_mobmgr;
    size_t mobsz = 32;
    spinlock_init(&initp->ks_lock);
    list_init(&initp->ks_tclst);
    initp->ks_tcnr = 0;
    initp->ks_msobnr = 0;
    initp->ks_msobche = NULL;
    for (uint_t i = 0; i < KOBLST_MAX; i++)
    {
        mo_mgr_list_init(&initp->ks_msoblst[i], mobsz);
        mobsz += 32;
    }
    return;
}