#include <hal/halstart.h>
#include <ldrtype.h>

extern kernel_desc_t kernel_info;

void init_hal() {}

void hal_start() {
  init_hal();
}