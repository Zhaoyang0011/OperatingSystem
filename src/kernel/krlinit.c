#include <kernel/krlinit.h>
#include <kernel/virmem.h>
#include <kernel/krlmempool.h>
#include <kernel/thread/scheduler.h>

void init_krl() {
  pvmspacdesc_init(&initial_pvmspacedesc);
  kmempool_init(&oskmempool);
  schedclass_t_init(&scheduler);
}