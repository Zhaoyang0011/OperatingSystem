#ifndef _SEMAPHORE_T_H
#define _SEMAPHORE_T_H

#include <spinlock.h>
#include <struct/list.h>

#define SEM_FLG_MUTEX 0
#define SEM_FLG_MULTI 1
#define SEM_MUTEX_ONE_LOCK 1
#define SEM_MULTI_LOCK 0

typedef struct kernel_wait_list {
  spinlock_t wl_lock;
  uint_t wl_tdnr;
  list_t wl_list;
} kwlist_t;

typedef struct semaphore {
  spinlock_t sem_lock;
  uint_t sem_flg;
  sint_t sem_count;
  kwlist_t sem_waitlst;
} sem_t;

void kwlist_init(kwlist_t *initp);
void kwlist_wait(kwlist_t *waitlist);
void kwlist_up(kwlist_t *wlst);
void kwlist_allup(kwlist_t *wlst);
//void krlwlst_add_thread(kwlist_t* wlst,thread_t* tdp);
//thread_t* krlwlst_del_thread(kwlist_t *wlst);

void krlsem_init(sem_t *initp);
void krlsem_set_sem(sem_t *setsem, uint_t flg, sint_t count);
void krlsem_down(sem_t *sem);
void krlsem_up(sem_t *sem);

#endif
