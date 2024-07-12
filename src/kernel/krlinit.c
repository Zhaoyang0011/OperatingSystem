#include <kernel/krlinit.h>
#include <kernel/virmem.h>
#include <kernel/krlmempool.h>

void init_krl() {
  pvmspacdesc_init(&initial_pvmspacedesc);
  kmempool_init(&oskmempool);
}