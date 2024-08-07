#include <ldrtype.h>
#include <memlayout.h>
#include <x86.h>

#include "ldr.h"

#define VIDEO_MEM 0xB8000

void ata_lba_read(uint32_t sector_start, uint32_t sector_count, uint32_t dest_adr) {

  __asm__ __volatile__("mov %0, %%eax\n\t" // 将 lba 参数加载到 eax 中
                       // LBA low
                       "mov %1, %%ecx\n\t"
                       "mov %2, %%edi\n\t"
                       "mov $0x1f3, %%dx\n\t"
                       "out %%al, %%dx\n\t"

                       // LBA mid
                       "mov $0x1f4, %%dx\n\t"
                       "shr $8, %%eax\n\t"
                       "out %%al, %%dx\n\t"

                       // LBA high
                       "mov $0x1f5, %%dx\n\t"
                       "shr $16, %%eax\n\t"
                       "out %%al, %%dx\n\t"

                       // 设置硬盘寄存器
                       "mov $0x1f6, %%dx\n\t"
                       "shr $8, %%eax\n\t"
                       "and $0x0f, %%al\n\t"
                       "or $0xe0, %%al\n\t"
                       "out %%al, %%dx\n\t"

                       // 发送读取命令
                       "mov $0x1f7, %%dx\n\t"
                       "mov $0x20, %%al\n\t"
                       "out %%al, %%dx\n\t"

                       // 读取扇区数据
                       ".next_sector:\n\t"
                       "push %%ecx\n\t"

                       "mov $0x1f7, %%dx\n\t"
                       ".check_hd:\n\t"
                       "in %%dx, %%al\n\t"
                       "test $8, %%al\n\t"
                       "jz .check_hd\n\t"

                       "movl $0x100, %%ecx\n\t" // 将 sector_count 加载到 ecx 中
                       "mov $0x1f0, %%dx\n\t"
                       "rep insw\n\t"

                       "pop %%ecx\n\t"
                       "loop .next_sector\n\t"
      :
      : "m"(sector_start), "m"(sector_count), "m"(dest_adr)
      : "eax", "edx", "ecx");
}

void waitdisk(void) {
  // Wait for disk ready.
  while ((inb(0x1F7) & 0xC0) != 0x40);
}

void kerror(char *err_msg, int length) {
  if (length > 0 || length <= 100) {
    for (int i = 0; i < length && err_msg[i]; ++i) {
      char *video_mem = (char *)VIDEO_MEM;
      video_mem[i * 2] = err_msg[i];
      video_mem[i * 2 + 1] = 15;
    }
  } else {
    kerror("invalid error massage!", 22);
  }
  while (TRUE);
}

// check cpu long mode
int chkcpu_long_mode() {
  cpuid_desc_t cpu_desc;
  cpu_desc.function_id = 0x80000000;

  if (cpuid(&cpu_desc) < 0x80000001)
    return FALSE;

  cpu_desc.function_id = 0x80000001;
  cpuid(&cpu_desc);

  if (cpu_desc.info[3] & (1 << 29))
    return TRUE;

  return FALSE;
}

// check and init kernel info
void chkini_kernel_info(kernel_desc_t *kernel_desc) {
  kernel_desc->kernel_start = KERNEL_START;
  if (kernel_desc->kernel_magic != ZHOS_MAGIC) {
    kerror("Invalid kernel magic number", 50);
  }

  if (kernel_desc->kernel_size == 0) {
    kerror("Incorrect kernel size", 50);
  }
//
//  if (!chkcpu_long_mode()) {
//	kerror("Your computer doest not support long mode", 50);
//  }

  kernel_desc->next_pg = P4K_ALIGN(KERNEL_START + kernel_desc->kernel_size);
}

void init_stack(kernel_desc_t *kernel_desc) {
  kernel_desc->stack_init_adr = STACK_PHYADR;
  kernel_desc->stack_size = STACK_SIZE;
}

e820_map_t *chk_memsize(e820_desc_t *e8p, uint64_t addr, uint64_t size) {
  uint64_t len = addr + size;
  if (e8p == NULL || e8p->e820_num == 0) {
    return NULL;
  }
  for (uint32_t i = 0; i < e8p->e820_num; i++) {
    if ((e8p->maps[i].type == RAM_USABLE) && (addr >= e8p->maps[i].addr) &&
        (len < (e8p->maps[i].addr + e8p->maps[i].size))) {
      return &e8p->maps[i];
    }
  }
  return NULL;
}

uint64_t get_memsize(e820_desc_t *e820_desc) {
  uint64_t size = 0;
  for (int i = 0; i < e820_desc->e820_num; ++i) {
    if (e820_desc->maps[i].type == RAM_USABLE) {
      size += e820_desc->maps[i].size;
    }
  }
  return size;
}

//
void init_memory_info(kernel_desc_t *kernel_desc) {
  e820_desc_t *e820_desc = (e820_desc_t *)E820_DESC;
  if (e820_desc->e820_num == 0) {
    kerror("No e820map", 20);
  }
  if (chk_memsize(e820_desc, 0x100000, 0x8000000) == 0) {
    kerror("Your computer is low on memory, the memory cannot be less than 128MB!", 20);
  }

  kernel_desc->mmap_adr = (uint64_t)(e820_desc->maps);
  kernel_desc->mmap_nr = (uint64_t)e820_desc->e820_num;
  kernel_desc->mmap_sz = (uint64_t)(e820_desc->e820_num * sizeof(e820_map_t));
  kernel_desc->mach_memsize = get_memsize(e820_desc);
}

void init_page_table(kernel_desc_t *kernel_desc) {
  uint64_t *p = (uint64_t *)KINITPAGE_PHYADR;
  uint64_t *pdpte = (uint64_t *)(KINITPAGE_PHYADR + 0x1000);
  uint64_t *pde = (uint64_t *)(KINITPAGE_PHYADR + 0x2000);

  for (int i = 0; i < PGENTY_SIZE; i++) {
    p[i] = 0;
    pdpte[i] = 0;
  }

  p[0] = (uint64_t)((uint32_t)pdpte | KPML4_RW | KPML4_P);
  p[(KRNL_VIRTUAL_ADDRESS_START >> 39) & 0x1ff] = (uint64_t)((uint32_t)pdpte | KPML4_RW | KPML4_P);

  uint64_t adr = 0;
  for (uint_t i = 0; i < 16; i++) {
    pdpte[i] = (uint64_t)((uint32_t)pde | KPDPTE_RW | KPDPTE_P);
    for (uint_t j = 0; j < PGENTY_SIZE; j++) {
      pde[j] = 0 | adr | KPDE_PS | KPDE_RW | KPDE_P;
      adr += (0x1 << KPDP_SHIFT);
    }
    pde = (uint64_t *)(((uint64_t)pde) + 0x1000);
  }
}