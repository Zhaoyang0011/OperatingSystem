#include "apic.h"
#include "gdtidt.h"
#include "../x86.h"
#include <type.h>
#include <console.h>
#include <kprint.h>
#include <hal/interrupt.h>

void init_interrupt() {
  init_idt();
  init_lapic();
}

void div_zero(uint_t faultnumb, void *krnlsframp) // rsi, rdi
{
}

bool_t krluserspace_accessfailed(addr_t fairvadrs) {
  return FALSE;
}

void hal_fault_allocator(uint_t faultnumb, void *krnlsframp) // rsi, rdi
{
  addr_t fairvadrs;
  kprint("faultnumb is :%d\n", faultnumb);
  if (faultnumb == 14) {    //获取缺页的地址
	fairvadrs = (addr_t)read_cr2();
	kprint("Seg fault:%x, address access forbidden!\n", fairvadrs);
	if (krluserspace_accessfailed(fairvadrs) != 0) {//处理缺页失败就死机
	  panic("Handle page fault error!\n");
	}
	//成功就返回
	return;
  }
}

sint_t hal_syscl_allocator(uint_t sys_nr, void *msgp) {
  return 0;
}

void hal_hwint_allocator(uint_t intnumb, void *krnlsframp) // rsi, rdi
{
  for (;;);
  return;
}