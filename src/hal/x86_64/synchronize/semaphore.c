#include <semaphore.h>

void kwlist_init(kwlist_t *initp) {
  spinlock_init(&initp->wl_lock);
  initp->wl_tdnr = 0;
  list_init(&initp->wl_list);
}

void kwlist_wait(kwlist_t *waitlist) {

}

void kwlist_up(kwlist_t *wlst) {

}

void kwlist_allup(kwlist_t *wlst) {

}

void krlsem_init(sem_t *initp) {
  spinlock_init(&initp->sem_lock);
  initp->sem_flg = 0;
  initp->sem_count = 0;

  kwlist_init(&initp->sem_waitlst);
}

void krlsem_set_sem(sem_t *setsem, uint_t flg, sint_t count) {
  cpuflg_t cpuflg;
  spinlock_cli(&setsem->sem_lock, &cpuflg);
  setsem->sem_flg = flg;
  setsem->sem_count = count;
  spinunlock_sti(&setsem->sem_lock, &cpuflg);
}

void krlsem_down(sem_t *sem) {
  cpuflg_t cpuflg;

  while (TRUE) {
    spinlock_cli(&sem->sem_lock, &cpuflg);
    sem->sem_count--;
    if (sem->sem_count > 0) {
      break;
    }
    // TODO implement kernel shcedule.
    kwlist_wait(&sem->sem_waitlst);
    spinunlock_sti(&sem->sem_lock, &cpuflg);
//        krlschedule_wait();
  }

  spinunlock_sti(&sem->sem_lock, &cpuflg);
}

void krlsem_up(sem_t *sem) {

}