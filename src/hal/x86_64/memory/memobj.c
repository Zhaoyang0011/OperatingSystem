#include <hal/memory/memgrob.h>
#include <hal/memory/memobject.h>

void momgrlist_init(momgrlist_t *initp, size_t mobsz)
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
        momgrlist_init(&initp->ks_msoblst[i], mobsz);
        mobsz += 32;
    }
    return;
}

// 分配内存对象的核心函数
void *mobj_alloc_core(size_t msz)
{
    momgrheader_t *pmomgrh = &memgrob.mo_mobmgr;
    cpuflg_t cpuflg;
    spinlock_cli(&pmomgrh->ks_lock, &cpuflg);

    void *retptr = NULL;
    momgrlist_t *pmgrlist = NULL;
    momgr_t *kmsp = NULL;

    spinunlock_sti(&pmomgrh->ks_lock, &cpuflg);
    return NULL;
}

// 内存对象分配接口
void *mobj_alloc(size_t msz)
{
    // 对于小于1 或者 大于2048字节的大小不支持 直接返回NULL表示失败
    if (1 > msz || 2048 < msz)
    {
        return NULL;
    }
    // 调用核心函数
    return mobj_alloc_core(msz);
}