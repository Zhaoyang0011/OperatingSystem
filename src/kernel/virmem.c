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

//检查kmvarsdsc_t结构
bool_t vma_kvmareadesc_is_ok(virmemspace_t *vmslocked, kvmareadesc_t *curr, addr_t start, size_t vassize) {
    kvmareadesc_t *nextkmvd = NULL;
    addr_t newend = start + (addr_t) vassize;
    //如果curr不是最后一个先检查当前kmvarsdsc_t结构
    if (list_is_last(&curr->kva_list, &vmslocked->vs_list) == FALSE) {//就获取curr的下一个kmvarsdsc_t结构
        nextkmvd = list_next_entry(curr, kvmareadesc_t, kva_list);
        //由系统动态决定分配虚拟空间的开始地址
        if (NULL == start) {//如果curr的结束地址加上分配的大小小于等于下一个kmvarsdsc_t结构的开始地址就返回curr
            if ((curr->kva_end + (addr_t) vassize) <= nextkmvd->kva_start) {
                return TRUE;
            }
        } else {//否则比较应用指定分配的开始、结束地址是不是在curr和下一个kmvarsdsc_t结构之间
            if ((curr->kva_end <= start) && (newend <= nextkmvd->kva_start)) {
                return TRUE;
            }
        }
    } else {//否则curr为最后一个kmvarsdsc_t结构
        if (NULL == start) {//curr的结束地址加上分配空间的大小是不是小于整个虚拟地址空间
            if ((curr->kva_end + (addr_t) vassize) < vmslocked->vs_isalcend) {
                return TRUE;
            }
        } else {//否则比较应用指定分配的开始、结束地址是不是在curr的结束地址和整个虚拟地址空间的结束地址之间
            if ((curr->kva_end <= start) && (newend < vmslocked->vs_isalcend)) {
                return TRUE;
            }
        }
    }
    return FALSE;
}

kvmareadesc_t *vma_find_kvmareadesc(virmemspace_t *vmslocked, addr_t start, size_t vassize) {
    kvmareadesc_t *kmvdcurrent = NULL, *curr = vmslocked->vs_currkmvdsc;
    addr_t newend = start + vassize;
    list_t *listpos = NULL;
    //分配的虚拟空间大小小于4KB不行
    if (0x1000 > vassize)
        return NULL;

    //将要分配虚拟地址空间的结束地址大于整个虚拟地址空间 不行
    if (newend > vmslocked->vs_isalcend)
        return NULL;

    if (NULL != curr && vma_kvmareadesc_is_ok(vmslocked, curr, start, vassize)) {//先检查当前kmvarsdsc_t结构行不行
        return curr;
    }
    //遍历virmemadrs_t中的所有的kmvarsdsc_t结构
    list_for_each(listpos, &vmslocked->vs_list) {
        curr = list_entry(listpos, kvmareadesc_t, kva_list);
        //检查每个kmvarsdsc_t结构
        if (vma_kvmareadesc_is_ok(vmslocked, curr, start, vassize)) {//如果符合要求就返回
            return curr;
        }
    }
    return NULL;
}

addr_t vma_new_varea_core(pvmspacdesc_t *mm, addr_t start, size_t vassize, uint64_t vaslimits, uint32_t vastype) {
    addr_t retadrs = NULL;
    kvmareadesc_t *newkmvd = NULL, *currkmvd = NULL;
    virmemspace_t *vma = &mm->msd_virmemadrs;
    spin_lock(&vma->vs_lock);

    do {
        //查找虚拟地址区间
        currkmvd = vma_find_kvmareadesc(vma, start, vassize);
        if (NULL == currkmvd) {
            retadrs = NULL;
            break;
        }
        //进行虚拟地址区间进行检查看能否复用这个数据结构
        if (((NULL == start) || (start == currkmvd->kva_end)) && (vaslimits == currkmvd->kva_limits)
            && (vastype == currkmvd->kva_maptype)) {//能复用的话，当前虚拟地址区间的结束地址返回
            retadrs = currkmvd->kva_end;
            //扩展当前虚拟地址区间的结束地址为分配虚拟地址区间的大小
            currkmvd->kva_end += vassize;
            vma->vs_currkmvdsc = currkmvd;
            break;
        }
        //建立一个新的kmvarsdsc_t虚拟地址区间结构
        newkmvd = new_kvmareadesc();
        if (NULL == newkmvd) {
            retadrs = NULL;
            break;
        }
        //如果分配的开始地址为NULL就由系统动态决定
        if (NULL == start) {//当然是接着当前虚拟地址区间之后开始
            newkmvd->kva_start = currkmvd->kva_end;
        } else {//否则这个新的虚拟地址区间的开始就是请求分配的开始地址
            newkmvd->kva_start = start;
        }
        //设置新的虚拟地址区间的结束地址
        newkmvd->kva_end = newkmvd->kva_start + vassize;
        newkmvd->kva_limits = vaslimits;
        newkmvd->kva_maptype = vastype;
        newkmvd->kva_mcstruct = vma;
        vma->vs_currkmvdsc = newkmvd;
        //将新的虚拟地址区间加入到virmemadrs_t结构中
        list_add(&newkmvd->kva_list, &currkmvd->kva_list);
        //看看新的虚拟地址区间是否是最后一个
        if (list_is_last(&newkmvd->kva_list, &vma->vs_list) == TRUE) {
            vma->vs_endkmvdsc = newkmvd;
        }
        //返回新的虚拟地址区间的开始地址
        retadrs = newkmvd->kva_start;

    } while (FALSE);

    spin_unlock(&vma->vs_lock);
    return retadrs;
}
//分配虚拟地址空间的接口
addr_t vma_new_varea(pvmspacdesc_t *mm, addr_t start, size_t vassize, uint64_t vaslimits, uint32_t vastype) {
    if (NULL == mm || 1 > vassize) {
        return NULL;
    }
    if (NULL != start) {//进行参数检查，开始地址要和页面（4KB）对齐，结束地址不能超过整个虚拟地址空间
        if (((start & 0xfff) != 0) || (0x1000 > start) || (USER_VIRTUAL_ADDRESS_END < (start + vassize))) {
            return NULL;
        }
    }
    //调用虚拟地址空间分配的核心函数
    return vma_new_varea_core(mm, start, VADSZ_ALIGN(vassize), vaslimits, vastype);
}