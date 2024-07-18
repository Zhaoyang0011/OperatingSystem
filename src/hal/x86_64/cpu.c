#include <hal/cpu.h>
#include "interrupt/apic.h"

uint32_t cur_cpuid() {
  return lapic_cpu[lapicid()];
}