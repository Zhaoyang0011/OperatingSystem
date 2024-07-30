#include <kernel/thread/scheduler.h>
#include <hal/cpu.h>
#include <console.h>

void thrdlst_init(thrdlst_t *initp) {
  list_init(&initp->tdl_lsth); //初始化挂载进程的链表
  initp->tdl_curruntd = NULL; //开始没有运行进程
  initp->tdl_nr = 0;  //开始没有进程
  return;
}

void schdata_init(schdata_t *initp) {
  spinlock_init(&initp->sda_lock);
  initp->sda_cpuid = cur_cpuid(); //获取CPU id
  initp->sda_schdflgs = NOTS_SCHED_FLGS;
  initp->sda_premptidx = 0;
  initp->sda_threadnr = 0;
  initp->sda_prityidx = 0;
  initp->sda_cpuidle = NULL; //开始没有空转进程和运行的进程
  initp->sda_currtd = NULL;
  for (uint_t ti = 0; ti < PRITY_MAX; ti++) {//初始化schdata_t结构中的每个thrdlst_t结构
    thrdlst_init(&initp->sda_thdlst[ti]);
  }
}

void schedclass_init(schedclass_t *initp) {
  spinlock_init(&initp->scls_lock);
  initp->scls_cpunr = CPUCORE_MAX;  //CPU最大个数
  initp->scls_threadnr = 0;   //开始没有进程
  initp->scls_threadid_inc = 0;
  for (uint_t si = 0; si < CPUCORE_MAX; si++) {
    //初始化osschedcls变量中的每个schdata_t
    schdata_init(&initp->scls_schda[si]);
  }
}

thread_t *scheduler_retn_idlethread() {
  uint_t cpuid = cur_cpuid();
  thread_t *idle = scheduler.scls_schda[cpuid].sda_cpuidle;
  if (idle == NULL) {
    //若调度数据结构中idle进程的指针为空，就出错死机
    panic("schdap->sda_cpuidle NULL");
  }
  return idle;
}

thread_t *scheduler_retn_curthread() {
  uint_t cpuid = cur_cpuid();
  //通过cpuid获取当前cpu的调度数据结构
  thread_t *cur_thread = scheduler.scls_schda[cpuid].sda_currtd;
  if (cur_thread == NULL) {
    //若调度数据结构中当前运行进程的指针为空，就出错死机
    panic("schdap->sda_currtd NULL");
  }
  return cur_thread;//返回当前运行的进程

}

thread_t *scheduler_select_thread() {
  thread_t *retthd, *tdtmp;
  cpuflg_t cufg;
  uint_t cpuid = cur_cpuid();
  schdata_t *schdap = &scheduler.scls_schda[cpuid];
  spinlock_cli(&schdap->sda_lock, &cufg);
  for (uint_t pity = 0; pity < PRITY_MAX; pity++) {//从最高优先级开始扫描
    if (schdap->sda_thdlst[pity].tdl_nr > 0) {//若当前优先级的进程链表不为空
      if (list_is_empty_careful(&(schdap->sda_thdlst[pity].tdl_lsth)) == FALSE) {//取出当前优先级进程链表下的第一个进程
        tdtmp = list_entry(schdap->sda_thdlst[pity].tdl_lsth.next, thread_t, td_list);
        list_del(&tdtmp->td_list);//脱链
        if (schdap->sda_thdlst[pity].tdl_curruntd != NULL) {//将这sda_thdlst[pity].tdl_curruntd的进程挂入链表尾
          list_add_tail(&(schdap->sda_thdlst[pity].tdl_curruntd->td_list), &schdap->sda_thdlst[pity].tdl_lsth);
        }
        schdap->sda_thdlst[pity].tdl_curruntd = tdtmp;
        retthd = tdtmp;//将选择的进程放入sda_thdlst[pity].tdl_curruntd中，并返回
        goto return_step;
      }
      if (schdap->sda_thdlst[pity].tdl_curruntd != NULL) {//若sda_thdlst[pity].tdl_curruntd不为空就直接返回它
        retthd = schdap->sda_thdlst[pity].tdl_curruntd;
        goto return_step;
      }
    }
  }
  //如果最后也没有找到进程就返回默认的空转进程
  schdap->sda_prityidx = PRITY_MIN;
  retthd = scheduler_retn_idlethread();
return_step:
  //解锁并返回进程
  spinunlock_sti(&schdap->sda_lock, &cufg);
  return retthd;

}

void scheduler_add_thread(thread_t *thdp) {
  uint_t cpuid = cur_cpuid();
  schdata_t *schdap = &scheduler.scls_schda[cpuid];
  cpuflg_t cufg;

  spinlock_cli(&schdap->sda_lock, &cufg);
  list_add(&thdp->td_list, &schdap->sda_thdlst[thdp->td_priority].tdl_lsth);
  schdap->sda_thdlst[thdp->td_priority].tdl_nr++;
  schdap->sda_threadnr++;
  spinunlock_sti(&schdap->sda_lock, &cufg);

  spinlock_cli(&scheduler.scls_lock, &cufg);
  scheduler.scls_threadnr++;
  spinunlock_sti(&scheduler.scls_lock, &cufg);
}

void schedule() {
  thread_t *prev = scheduler_retn_curthread(),  //返回当前运行进程
  *next = scheduler_select_thread();            //选择下一个运行的进程
//  kprint("current: %p, next: %p\n", prev, next);
  switch_context(next, prev);                   //从当前进程切换到下一个进程
}