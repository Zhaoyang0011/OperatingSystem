#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define ALIGN(x, a) (((x) + (a)-1) & ~((a)-1))
#define P4K_ALIGN(x) ALIGN(x, 0x1000)
#define ZHOS_MAGIC                                                  \
  (u_int64_t)((((u_int64_t)'Z') << 56) | (((u_int64_t)'H') << 48) | \
              (((u_int64_t)'O') << 40) | (((u_int64_t)'S') << 32) | \
              (((u_int64_t)'M') << 24) | (((u_int64_t)'A') << 16) | \
              (((u_int64_t)'C') << 8) | ((u_int64_t)'H'))

typedef u_int64_t uint64_t;

typedef struct kernel_desc {
  uint64_t kernel_magic;  // magic of os
  uint64_t kernel_start;  // starting address of kernel
  uint64_t kernel_size;   // kernel size
  uint64_t init_stack;    // kernel stack address
  uint64_t stack_sz;      // kernel stack size
  uint64_t mach_memsize;  // memory size of machine
  uint64_t mmap_adr;      // address of e820 array
  uint64_t mmap_nr;       // number of e820 entry
  uint64_t mmap_sz;       // size of e820 array
  uint64_t mp_desc_arr;   // start address of memory page descriptor array
  uint64_t mp_desc_nr;    // number of memory page descriptors
  uint64_t mp_desc_sz;    // size of memory page descriptor array
  uint64_t ma_phyadr;     // physical address of memory area array
  uint64_t ma_nr;
  uint64_t ma_sz;
  uint64_t next_pg;  // next paging address
} __attribute__((packed)) kernel_desc_t;

void set_kernel_desc(kernel_desc_t* pdesc, uint64_t kernel_size) {
  memset(pdesc, 0, sizeof(kernel_desc_t));
  pdesc->kernel_magic = ZHOS_MAGIC;
  pdesc->kernel_size = kernel_size;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Only support 2 parameters\n");
    return -1;
  }
  char* filename = argv[1];
  int ifd;
  if ((ifd = open(filename, O_RDONLY)) < 0) {
    printf("Open file error!\n");
    return -1;
  }
  int kernel_size = 0;
  kernel_size = lseek(ifd, 0, SEEK_END);
  printf("Kernel size is %d.\n", kernel_size);
  if (kernel_size < 0) {
    printf("Lseek error.\n");
    return -1;
  }

  char buf[4096];
  memset(buf, 0, sizeof(buf));
  kernel_desc_t desc;
  set_kernel_desc(&desc, kernel_size);

  int ofd;

  if ((ofd = open(argv[2], O_RDWR | O_TRUNC | O_CREAT)) < 0) {
    printf("Open file error!\n");
    return -1;
  }
  write(ofd, &desc, sizeof(desc));
  write(ofd, buf, 0x1000 - sizeof(desc));

  lseek(ifd, 0, SEEK_SET);
  int offset = 0;
  int read_size = 0;
  do {
    read_size = read(ifd, buf, sizeof(buf));
    write(ofd, buf, read_size);
  } while (read_size != 0);

  printf("Succefully packed kernel!\n");
  close(ifd);
  close(ofd);

  return 0;
}