#include <kernel/thread/thread.h>
#include <kernel/krlmempool.h>
#include <string.h>

uint_t hal_retn_cpuid() {
  return 0;
}
//返回进程id其实就thread_t结构的地址
uint_t krlretn_thread_id(thread_t *tdp) {
  return (uint_t)tdp;
}
//初始化thread_t结构
void thread_init(thread_t *initp) {
  spin_lock(&initp->td_lock);
  list_init(&initp->td_list);
  initp->td_flgs = TDFLAG_FREE;
  initp->td_stus = TDSTUS_NEW;//进程状态为新建
  initp->td_cpuid = hal_retn_cpuid();
  initp->td_id = krlretn_thread_id(initp);
  initp->td_tick = 0;
  initp->td_privilege = PRILG_USR;//普通进程权限
  initp->td_priority = PRITY_MIN;//最高优先级
  initp->td_runmode = 0;
  initp->td_krlstktop = NULL;
  initp->td_krlstkstart = NULL;
  initp->td_usrstktop = NULL;
  initp->td_usrstkstart = NULL;
  initp->td_mmdsc = &initial_pvmspacedesc;//指向默认的地址空间结构

  context_init(&initp->td_context);
}

//创建thread_t结构
thread_t *krlnew_thread_desc() {
  //分配thread_t结构大小的内存空间
  thread_t *rettdp = (thread_t *)(krlnew((size_t)(sizeof(thread_t))));
  if (rettdp == NULL) {
    return NULL;
  }
  //初始化刚刚分配的thread_t结构
  thread_init(rettdp);
  return rettdp;
}

void krlthread_stack_init(thread_t *p_thread, void *runadr, uint_t cpuflags) {
  //处理栈顶16字节对齐
  p_thread->td_krlstktop &= (~0xf);
  p_thread->td_usrstktop &= (~0xf);
  //内核栈顶减去intstkregs_t结构的大小
  krlstkregs_t *arp = (krlstkregs_t *)(p_thread->td_krlstktop - sizeof(krlstkregs_t));

#ifdef x86_64
  //把intstkregs_t结构的空间初始化为0
  memset((void *)arp, 0, sizeof(krlstkregs_t));
  //rip寄存器的值设为程序运行首地址
  arp->r_rip_old = (uint_t)runadr;
  //cs寄存器的值设为内核代码段选择子
  arp->r_cs_old = K_CS_IDX;
  arp->r_rflgs = cpuflags;
  //返回进程的内核栈
  arp->r_rsp_old = p_thread->td_krlstktop;
  arp->r_ss_old = 0;
  //其它段寄存器的值设为内核数据段选择子
  arp->r_ds = K_DS_IDX;
  arp->r_es = K_DS_IDX;
  arp->r_fs = K_DS_IDX;
  arp->r_gs = K_DS_IDX;
#endif

  //设置进程下一次运行的地址为runadr
  p_thread->td_context.ctx_nextrip = (uint_t)runadr;
  //设置进程下一次运行的栈地址为arp
  p_thread->td_context.ctx_nextrsp = (uint_t)arp;
}

thread_t *krl_new_krl_thread_core(void *filerun,
                                  uint_t flg,
                                  uint_t prilg,
                                  uint_t prity,
                                  size_t usrstksz,
                                  size_t krlstksz) {
  thread_t *ret_td = NULL;
  bool_t acs = FALSE;
  addr_t krlstkadr = NULL;
  //分配内核栈空间
  krlstkadr = krlnew(krlstksz);
  if (krlstkadr == NULL) {
    return NULL;
  }
  //建立thread_t结构体的实例变量
  ret_td = krlnew_thread_desc();
  if (ret_td == NULL) {//创建失败必须要释放之前的栈空间
    acs = krldelete(krlstkadr, krlstksz);
    if (acs == FALSE) {
      return NULL;
    }
    return NULL;
  }
  //设置进程权限
  ret_td->td_privilege = prilg;
  //设置进程优先级
  ret_td->td_priority = prity;

  //设置进程的内核栈顶和内核栈开始地址
  ret_td->td_krlstktop = krlstkadr + (addr_t)(krlstksz - 1);
  ret_td->td_krlstkstart = krlstkadr;

  //初始化进程的内核栈
  krlthread_stack_init(ret_td, filerun, KMOD_EFLAGS);
  //加入进程调度系统
//  krlschdclass_add_thread(ret_td);
  //返回进程指针
  return ret_td;
}

//TODO create user thread
thread_t *krl_new_user_thread_core(void *filerun,
                                   uint_t flg,
                                   uint_t prilg,
                                   uint_t prity,
                                   size_t usrstksz,
                                   size_t krlstksz) {
  return NULL;
}

thread_t *krl_new_thread(void *filerun, uint_t flg, uint_t prilg, uint_t prity, size_t usrstksz, size_t krlstksz) {
  size_t tustksz = usrstksz, tkstksz = krlstksz;
  //对参数进行检查，不合乎要求就返回NULL表示创建失败
  if (filerun == NULL || usrstksz > DAFT_TDUSRSTKSZ || krlstksz > DAFT_TDKRLSTKSZ) {
    return NULL;
  }
  if ((prilg != PRILG_USR && prilg != PRILG_SYS) || (prity >= PRITY_MAX)) {
    return NULL;
  }
  //进程应用程序栈大小检查，大于默认大小则使用默认大小
  if (usrstksz < DAFT_TDUSRSTKSZ) {
    tustksz = DAFT_TDUSRSTKSZ;
  }
  //进程内核栈大小检查，大于默认大小则使用默认大小
  if (krlstksz < DAFT_TDKRLSTKSZ) {
    tkstksz = DAFT_TDKRLSTKSZ;
  }
  //是否建立内核进程
  if (KERNTHREAD_FLG == flg) {
    return krl_new_krl_thread_core(filerun, flg, prilg, prity, tustksz, tkstksz);
  }
    //是否建立普通进程
  else if (USERTHREAD_FLG == flg) {
    return krl_new_user_thread_core(filerun, flg, prilg, prity, tustksz, tkstksz);
  }
  return NULL;
}