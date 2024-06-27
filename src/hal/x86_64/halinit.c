#include <hal/halinit.h>
#include <hal/interrupt.h>
#include <hal/mp.h>
#include <hal/memory/memory.h>
#include <ldrtype.h>

void init_hal() {
  init_mp();
  init_memory();
  init_interrupt();
}