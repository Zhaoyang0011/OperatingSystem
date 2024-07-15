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

void pvmspacdesc_init(pvmspacdesc_t *initp) {
  if (NULL == initp) {
    return;
  }
  spinlock_init(&initp->msd_lock);
  list_init(&initp->msd_list);
  initp->msd_flag = 0;
  initp->msd_stus = 0;
  initp->msd_scount = 0;
  krlsem_init(&initp->msd_sem);
  krlsem_set_sem(&initp->msd_sem, SEM_FLG_MUTEX, SEM_MUTEX_ONE_LOCK);

  // TODO implement mmu related
  mmudesc_init(&initp->msd_mmu);
  virmemspace_init(&initp->msd_virmemadrs);
  initp->msd_stext = 0;
  initp->msd_etext = 0;
  initp->msd_sdata = 0;
  initp->msd_edata = 0;
  initp->msd_sbss = 0;
  initp->msd_ebss = 0;
  initp->msd_sbrk = 0;
  initp->msd_ebrk = 0;
}

kvmareadesc_t *new_kvmareadesc() {
  kvmareadesc_t *kmvdc = NULL;
  kmvdc = (kvmareadesc_t *)kmobj_alloc(sizeof(kvmareadesc_t));
  if (NULL == kmvdc)
    return NULL;

  kvmareadesc_init(kmvdc);
  return kmvdc;
}

bool_t del_kvmareadesc(kvmareadesc_t *delkmvd) {
  if (NULL == delkmvd) {
    return FALSE;
  }
  return kmobj_free((void *)delkmvd, sizeof(kvmareadesc_t));
}

//检查kmvarsdsc_t结构
bool_t vms_kvmareadesc_is_ok(virmemspace_t *vmslocked, kvmareadesc_t *curr, addr_t start, size_t vassize) {
  kvmareadesc_t *nextkmvd = NULL;
  addr_t newend = start + (addr_t)vassize;
  //如果curr不是最后一个先检查当前kmvarsdsc_t结构
  if (list_is_last(&curr->kva_list, &vmslocked->vs_list) == FALSE) {//就获取curr的下一个kmvarsdsc_t结构
    nextkmvd = list_next_entry(curr, kvmareadesc_t, kva_list);
    //由系统动态决定分配虚拟空间的开始地址
    if (NULL == start) {//如果curr的结束地址加上分配的大小小于等于下一个kmvarsdsc_t结构的开始地址就返回curr
      if ((curr->kva_end + (addr_t)vassize) <= nextkmvd->kva_start) {
        return TRUE;
      }
    } else {//否则比较应用指定分配的开始、结束地址是不是在curr和下一个kmvarsdsc_t结构之间
      if ((curr->kva_end <= start) && (newend <= nextkmvd->kva_start)) {
        return TRUE;
      }
    }
  } else {//否则curr为最后一个kmvarsdsc_t结构
    if (NULL == start) {//curr的结束地址加上分配空间的大小是不是小于整个虚拟地址空间
      if ((curr->kva_end + (addr_t)vassize) < vmslocked->vs_isalcend) {
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

kvmareadesc_t *vms_find_kvmareadesc(virmemspace_t *vmslocked, addr_t start, size_t vassize) {
  kvmareadesc_t *kmvdcurrent = NULL, *curr = vmslocked->vs_currkmvdsc;
  addr_t newend = start + vassize;
  list_t *listpos = NULL;
  //分配的虚拟空间大小小于4KB不行
  if (0x1000 > vassize)
    return NULL;

  //将要分配虚拟地址空间的结束地址大于整个虚拟地址空间 不行
  if (newend > vmslocked->vs_isalcend)
    return NULL;

  if (NULL != curr && vms_kvmareadesc_is_ok(vmslocked, curr, start, vassize)) {//先检查当前kmvarsdsc_t结构行不行
    return curr;
  }
  //遍历virmemadrs_t中的所有的kmvarsdsc_t结构
  list_for_each(listpos, &vmslocked->vs_list) {
    curr = list_entry(listpos, kvmareadesc_t, kva_list);
    //检查每个kmvarsdsc_t结构
    if (vms_kvmareadesc_is_ok(vmslocked, curr, start, vassize)) {//如果符合要求就返回
      return curr;
    }
  }
  return NULL;
}

addr_t vms_new_varea_core(pvmspacdesc_t *mm, addr_t start, size_t vassize, uint64_t vaslimits, uint32_t vastype) {
  addr_t retadrs = NULL;
  kvmareadesc_t *newkmvd = NULL, *currkmvd = NULL;
  virmemspace_t *vma = &mm->msd_virmemadrs;
  spin_lock(&vma->vs_lock);

  do {
    //查找虚拟地址区间
    currkmvd = vms_find_kvmareadesc(vma, start, vassize);
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
addr_t vms_new_varea(pvmspacdesc_t *mm, addr_t start, size_t vassize, uint64_t vaslimits, uint32_t vastype) {
  if (NULL == mm || 1 > vassize) {
    return NULL;
  }
  if (NULL != start) {//进行参数检查，开始地址要和页面（4KB）对齐，结束地址不能超过整个虚拟地址空间
    if (((start & 0xfff) != 0) || (0x1000 > start) || (USER_VIRTUAL_ADDRESS_END < (start + vassize))) {
      return NULL;
    }
  }
  //调用虚拟地址空间分配的核心函数
  return vms_new_varea_core(mm, start, VADSZ_ALIGN(vassize), vaslimits, vastype);
}

kvmareadesc_t *vms_free_find_kvmareadesc(virmemspace_t *vmslocked, addr_t start, size_t vassize) {
  kvmareadesc_t *curr = vmslocked->vs_currkmvdsc;
  addr_t end = start + vassize;
  list_t *listpos = NULL;

  if (NULL != curr) {
    //释放的虚拟地址空间落在了当前kmvarsdsc_t结构表示的虚拟地址区间
    if ((curr->kva_start) <= start && (end <= curr->kva_end)) {
      return curr;
    }
  }
  //遍历所有的kmvarsdsc_t结构
  list_for_each(listpos, &vmslocked->vs_list) {
    curr = list_entry(listpos, kvmareadesc_t, kva_list);
    //释放的虚拟地址空间是否落在了其中的某个kmvarsdsc_t结构表示的虚拟地址区间
    if ((start >= curr->kva_start) && (end <= curr->kva_end)) {
      return curr;
    }
  }
  return NULL;
}

//释放虚拟地址空间的核心函数
bool_t vms_free_vadrs_core(pvmspacdesc_t *mm, addr_t start, size_t vassize) {
  bool_t rets = FALSE;
  kvmareadesc_t *newkmvd = NULL, *delkmvd = NULL;
  virmemspace_t *vma = &mm->msd_virmemadrs;
  spin_lock(&vma->vs_lock);
  //查找要释放虚拟地址空间的kmvarsdsc_t结构
  delkmvd = vms_free_find_kvmareadesc(vma, start, vassize);
  if (NULL == delkmvd) {
    rets = FALSE;
    goto out;
  }
  //第一种情况要释放的虚拟地址空间正好等于查找的kmvarsdsc_t结构
  if ((delkmvd->kva_start == start) && (delkmvd->kva_end == (start + (addr_t)vassize))) {
    //脱链
    list_del(&delkmvd->kva_list);
    //删除kmvarsdsc_t结构
    del_kvmareadesc(delkmvd);
    vma->vs_kmvdscnr--;
    rets = TRUE;
    goto out;
  }
  //第二种情况要释放的虚拟地址空间是在查找的kmvarsdsc_t结构的上半部分
  if ((delkmvd->kva_start == start)
      && (delkmvd->kva_end > (start + (addr_t)vassize))) {    //所以直接把查找的kmvarsdsc_t结构的开始地址设置为释放虚拟地址空间的结束地址
    delkmvd->kva_start = start + (addr_t)vassize;
    rets = TRUE;
    goto out;
  }
  //第三种情况要释放的虚拟地址空间是在查找的kmvarsdsc_t结构的下半部分
  if ((delkmvd->kva_start < start)
      && (delkmvd->kva_end == (start + (addr_t)vassize))) {//所以直接把查找的kmvarsdsc_t结构的结束地址设置为释放虚拟地址空间的开始地址
    delkmvd->kva_end = start;
    rets = TRUE;
    goto out;
  }
  //第四种情况要释放的虚拟地址空间是在查找的kmvarsdsc_t结构的中间
  if ((delkmvd->kva_start < start)
      && (delkmvd->kva_end > (start + (addr_t)vassize))) {//所以要再新建一个kmvarsdsc_t结构来处理释放虚拟地址空间之后的下半虚拟部分地址空间
    newkmvd = new_kvmareadesc();
    if (NULL == newkmvd) {
      rets = FALSE;
      goto out;
    }
    //让新的kmvarsdsc_t结构指向查找的kmvarsdsc_t结构的后半部分虚拟地址空间
    newkmvd->kva_end = delkmvd->kva_end;
    newkmvd->kva_start = start + (addr_t)vassize;
    //和查找到的kmvarsdsc_t结构保持一致
    newkmvd->kva_limits = delkmvd->kva_limits;
    newkmvd->kva_maptype = delkmvd->kva_maptype;
    newkmvd->kva_mcstruct = vma;
    delkmvd->kva_end = start;
    //加入链表
    list_add(&newkmvd->kva_list, &delkmvd->kva_list);
    vma->vs_kmvdscnr++;
    //是否为最后一个kmvarsdsc_t结构
    if (list_is_last(&newkmvd->kva_list, &vma->vs_list) == TRUE) {
      vma->vs_endkmvdsc = newkmvd;
      vma->vs_currkmvdsc = newkmvd;
    } else {
      vma->vs_currkmvdsc = newkmvd;
    }
    rets = TRUE;
    goto out;
  }
  rets = FALSE;
out:
  spin_unlock(&vma->vs_lock);
  return rets;
}

//释放虚拟地址空间的接口
bool_t vms_free_vadrs(pvmspacdesc_t *mm, addr_t start, size_t vassize) {    //对参数进行检查
  if (NULL == mm || 1 > vassize || NULL == start) {
    return FALSE;
  }
  //调用核心处理函数
  return vms_free_vadrs_core(mm, start, VADSZ_ALIGN(vassize));
}

kvmareadesc_t *vms_map_find_kvmareadesc(virmemspace_t *vmslocked, addr_t fairvadrs) {
  kvmareadesc_t *curr = vmslocked->vs_currkmvdsc;
  list_t *listpos = NULL;

  if (curr != NULL && (fairvadrs >= curr->kva_start || fairvadrs < curr->kva_end)) {
    return curr;
  }
  //遍历所有的kmvarsdsc_t结构
  list_for_each(listpos, &vmslocked->vs_list) {
    curr = list_entry(listpos, kvmareadesc_t, kva_list);
    //释放的虚拟地址空间是否落在了其中的某个kmvarsdsc_t结构表示的虚拟地址区间
    if ((fairvadrs >= curr->kva_start) && (fairvadrs <= curr->kva_end)) {
      return curr;
    }
  }
  return NULL;
}

kvmemcbox_t *knl_get_kvmemcbox() {
  kvmemcbox_t *ret = kmobj_alloc(sizeof(kvmemcbox_t));
  return ret;
}

kvmemcbox_t *vma_map_retn_kvmemcbox(kvmareadesc_t *kmvd) {
  kvmemcbox_t *kmbox = NULL;
  //如果kmvarsdsc_t结构中已经存在了kvmemcbox_t结构，则直接返回
  if (NULL != kmvd->kva_kvmbox) {
    return kmvd->kva_kvmbox;
  }
  //新建一个kvmemcbox_t结构
  kmbox = knl_get_kvmemcbox();
  if (NULL == kmbox) {
    return NULL;
  }
  //指向这个新建的kvmemcbox_t结构
  kmvd->kva_kvmbox = kmbox;
  return kmvd->kva_kvmbox;
}

// TODO implement map virtual memory
sint32_t vms_map_virtual_memory(pvmspacdesc_t *procmem, addr_t fairvadrs) {
  virmemspace_t *vms = &procmem->msd_virmemadrs;
  sint32_t ret = 0;

  spin_lock(&vms->vs_lock);

  kvmareadesc_t *vmarea = vms_map_find_kvmareadesc(vms, fairvadrs);
  if (vmarea == NULL) {
    ret = 0;
    goto out;
  }

out:
  spin_unlock(&vms->vs_lock);
  return ret;
}

sint32_t krluserspace_accessfailed(addr_t fairvadrs) {
  if ((0x1000 > fairvadrs) || (USER_VIRTUAL_ADDRESS_END < fairvadrs))
    return FALSE;
  //TODO get current process memory information
  pvmspacdesc_t *procmem = NULL;
  if (procmem == NULL)
    return FALSE;
  return vms_map_virtual_memory(procmem, fairvadrs);
}