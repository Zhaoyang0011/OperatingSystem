#include <kernel/virmem.h>
#include <hal/memory/memobject.h>

void kvmareadesc_init(kvmareadesc_t *initp) {
    spinlock_init(&initp->kva_lock);
    initp->kva_maptype = 0;
    list_init(&initp->kva_list);
    initp->kva_flgs = 0;
    initp->kva_limits = 0;
    initp->kva_mcstruct = NULL;
    initp->kva_start = 0;
    initp->kva_end = 0;
    initp->kva_kvmbox = NULL;
    initp->kva_kvmcobj = NULL;
}

kvmareadesc_t *new_kvmareadesc() {
    kvmareadesc_t *kmvdc = NULL;
    kmvdc = (kvmareadesc_t *) mobj_alloc(sizeof(kvmareadesc_t));
    if (NULL == kmvdc)
        return NULL;

    kvmareadesc_init(kmvdc);
    return kmvdc;
}

bool_t del_kvmareadesc(kvmareadesc_t *delkmvd) {
    if (NULL == delkmvd) {
        return FALSE;
    }
    return mobj_free((void *) delkmvd, sizeof(kvmareadesc_t));
}

bool_t virmemspace_init(virmemspace_t *vma) {
    kvmareadesc_t *kmvdc = NULL, *stackkmvdc = NULL;
    //分配一个kmvarsdsc_t
    kmvdc = new_kvmareadesc();
    if (NULL == kmvdc)
        return FALSE;

    //分配一个栈区的kmvarsdsc_t
    stackkmvdc = new_kvmareadesc();
    if (NULL == stackkmvdc) {
        del_kvmareadesc(kmvdc);
        return FALSE;
    }

    //虚拟区间开始地址0x1000
    kmvdc->kva_start = USER_VIRTUAL_ADDRESS_START + 0x1000;
    //虚拟区间结束地址0x5000
    kmvdc->kva_end = kmvdc->kva_start + 0x4000;
    kmvdc->kva_mcstruct = vma;

    //栈虚拟区间开始地址0x1000USER_VIRTUAL_ADDRESS_END - 0x40000000
    stackkmvdc->kva_start = P4K_ALIGN(USER_VIRTUAL_ADDRESS_END - 0x40000000);
    //栈虚拟区间结束地址0x1000USER_VIRTUAL_ADDRESS_END
    stackkmvdc->kva_end = USER_VIRTUAL_ADDRESS_END;
    stackkmvdc->kva_mcstruct = vma;

    spin_lock(&vma->vs_lock);
    vma->vs_isalcstart = USER_VIRTUAL_ADDRESS_START;
    vma->vs_isalcend = USER_VIRTUAL_ADDRESS_END;
    //设置虚拟地址空间的开始区间为kmvdc
    vma->vs_startkmvdsc = kmvdc;
    //设置虚拟地址空间的开始区间为栈区
    vma->vs_endkmvdsc = stackkmvdc;
    //加入链表
    list_add_tail(&kmvdc->kva_list, &vma->vs_list);
    list_add_tail(&stackkmvdc->kva_list, &vma->vs_list);
    //计数加2
    vma->vs_kmvdscnr += 2;
    spin_unlock(&vma->vs_lock);
    return TRUE;
}