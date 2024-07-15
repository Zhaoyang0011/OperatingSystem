#ifndef _THREAD_H
#define _THREAD_H

#include <kernel/thread/context.h>
#include <struct/list.h>
#include <spinlock.h>
#include <kernel/virmem.h>

#define TDSTUS_RUN 0
#define TDSTUS_SLEEP 3
#define TDSTUS_WAIT 4
#define TDSTUS_NEW 5
#define TDSTUS_ZOMB 6

#define TDFLAG_FREE (1)
#define TDFLAG_BUSY (2)

#define TDRUN_TICK 20

#define PRITY_MAX 64
#define PRITY_MIN 0
#define PRILG_SYS 0
#define PRILG_USR 5

#define MICRSTK_MAX 4

#define THREAD_MAX (4)

#define KERNTHREAD_FLG 0
#define USERTHREAD_FLG 3

#ifdef x86_64

#define DAFT_TDUSRSTKSZ 0x8000
#define DAFT_TDKRLSTKSZ 0x8000

#define TD_HAND_MAX 8
#define DAFT_SPSR 0x10
#define DAFT_CPSR 0xd3
#define DAFT_CIDLESPSR 0x13

#endif

#define K_CS_IDX    0x08
#define K_DS_IDX    0x10
#define U_CS_IDX    0x1b
#define U_DS_IDX    0x23
#define K_TAR_IDX   0x28
#define UMOD_EFLAGS 0x1202
#define KMOD_EFLAGS    0x202

typedef struct thread {
  spinlock_t td_lock;       //进程的自旋锁
  list_t td_list;           //进程链表
  uint_t td_flgs;           //进程的标志
  uint_t td_stus;           //进程的状态
  uint_t td_cpuid;          //进程所在的CPU的id
  uint_t td_id;             //进程的id
  uint_t td_tick;           //进程运行了多少tick
  uint_t td_privilege;      //进程的权限
  uint_t td_priority;       //进程的优先级
  uint_t td_runmode;        //进程的运行模式
  addr_t td_krlstktop;      //应用程序内核栈顶地址
  addr_t td_krlstkstart;    //应用程序内核栈开始地址
  addr_t td_usrstktop;      //应用程序栈顶地址
  addr_t td_usrstkstart;    //应用程序栈开始地址
  pvmspacdesc_t *td_mmdsc;  //地址空间结构
  context_t td_context;     //机器上下文件结构
//    objnode_t*  td_handtbl[TD_HAND_MAX];//打开的对象数组
} thread_t;

thread_t *new_cpuidle_thread();
thread_t *krl_new_thread(void *filerun, uint_t flg, uint_t prilg, uint_t prity, size_t usrstksz, size_t krlstksz);

#endif