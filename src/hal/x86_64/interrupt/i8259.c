#include "i8259.h"
#include "../x86.h"

void disable_i8259() {
  outb(PIC1_DATA, 0xff);
  outb(PIC2_DATA, 0xff);
}