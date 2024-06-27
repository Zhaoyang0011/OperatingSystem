#include <kernel/thread.h>

//thread_t *krl_new_krl_thread_core(void *filerun,
//                                  uint_t flg,
//                                  uint_t prilg,
//                                  uint_t prity,
//                                  size_t usrstksz,
//                                  size_t krlstksz) {
//
//    thread_t *ret_td = NULL;
//    bool_t acs = FALSE;
//    addr_t krlstkadr = NULL;
//    //分配内核栈空间
//    krlstkadr = krlnew(krlstksz);
//    if (krlstkadr == NULL) {
//        return NULL;
//    }
//    //建立thread_t结构体的实例变量
//    ret_td = krlnew_thread_dsc();
//    if (ret_td == NULL) {//创建失败必须要释放之前的栈空间
//        acs = krldelete(krlstkadr, krlstksz);
//        if (acs == FALSE) {
//            return NULL;
//        }
//        return NULL;
//    }
//    //设置进程权限
//    ret_td->td_privilege = prilg;
//    //设置进程优先级
//    ret_td->td_priority = prity;
//    //设置进程的内核栈顶和内核栈开始地址
//    ret_td->td_krlstktop = krlstkadr + (adr_t)(krlstksz - 1);
//    ret_td->td_krlstkstart = krlstkadr;
//    //初始化进程的内核栈
//    krlthread_kernstack_init(ret_td, filerun, KMOD_EFLAGS);
//    //加入进程调度系统
//    krlschdclass_add_thread(ret_td);
//    //返回进程指针
//    return ret_td;
//}
//
//thread_t *krl_new_user_thread_core(void *filerun,
//                                   uint_t flg,
//                                   uint_t prilg,
//                                   uint_t prity,
//                                   size_t usrstksz,
//                                   size_t krlstksz) {
//    return NULL;
//}
//
//thread_t *krl_new_thread(void *filerun, uint_t flg, uint_t prilg, uint_t prity, size_t usrstksz, size_t krlstksz) {
//    size_t tustksz = usrstksz, tkstksz = krlstksz;
//    //对参数进行检查，不合乎要求就返回NULL表示创建失败
//    if (filerun == NULL || usrstksz > DAFT_TDUSRSTKSZ || krlstksz > DAFT_TDKRLSTKSZ) {
//        return NULL;
//    }
//    if ((prilg != PRILG_USR && prilg != PRILG_SYS) || (prity >= PRITY_MAX)) {
//        return NULL;
//    }
//    //进程应用程序栈大小检查，大于默认大小则使用默认大小
//    if (usrstksz < DAFT_TDUSRSTKSZ) {
//        tustksz = DAFT_TDUSRSTKSZ;
//    }
//    //进程内核栈大小检查，大于默认大小则使用默认大小
//    if (krlstksz < DAFT_TDKRLSTKSZ) {
//        tkstksz = DAFT_TDKRLSTKSZ;
//    }
//    //是否建立内核进程
//    if (KERNTHREAD_FLG == flg) {
//        return krl_new_krl_thread_core(filerun, flg, prilg, prity, tustksz, tkstksz);
//    }
//        //是否建立普通进程
//    else if (USERTHREAD_FLG == flg) {
//        return krl_new_user_thread_core(filerun, flg, prilg, prity, tustksz, tkstksz);
//    }
//    return NULL;
//}