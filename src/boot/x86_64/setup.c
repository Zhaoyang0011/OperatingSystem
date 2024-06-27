#include "ldr.h"
#include <type.h>

#define VIDEO_MEM 0xB8000

void setup_die() {
  char *video_mem = (char *)VIDEO_MEM;
  video_mem[0] = 'D';
  video_mem[1] = 15;
  video_mem[2] = 'I';
  video_mem[3] = 15;
  video_mem[4] = 'E';
  video_mem[5] = 15;
  while (TRUE);
}

volatile int bp = 0;

void boot_processor() {
  ata_lba_read(12, 8, KERNEL_START + KERNEL_DESC_OFF);

  kernel_desc_t *kernel_desc = (kernel_desc_t *)(KERNEL_START + KERNEL_DESC_OFF);

  chkini_kernel_info(kernel_desc);
  init_stack(kernel_desc);
  init_memory_info(kernel_desc);
  init_page_table(kernel_desc);

  uint32_t sector_count = kernel_desc->kernel_size / 512;
  if (kernel_desc->kernel_size % 512 != 0)
	sector_count++;
  kernel_desc->bp = 1;
  ata_lba_read(20, sector_count, KERNEL_START);
}

void app_processor() {

}

/**
 * This is the main function of the setup.
 * It will call other functions that prepares our device information.
 */
void setup_main() {
  if (bp == 0) {
	boot_processor();
	bp = 1;
  } else
	app_processor();
}