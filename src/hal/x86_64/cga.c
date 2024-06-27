#include "cga.h"
#include "io.h"
#include <console.h>
#include <string.h>

#define BACKSPACE 0x100
#define CRTPORT 0x3d4

#define CGA_CONSOLE ((uint16_t *)P2V(0xb8000))

static spinlock_t cga_lock;

static void cga_putc(int c) {
  int pos;

  // Cursor position: col + 80*row.
  outb(CRTPORT, 14);
  pos = inb(CRTPORT + 1) << 8;
  outb(CRTPORT, 15);
  pos |= inb(CRTPORT + 1);

  if (c == '\n')
	pos += 80 - pos % 80;
  else if (c == BACKSPACE) {
	if (pos > 0)
	  --pos;
  } else
	CGA_CONSOLE[pos++] = (c & 0xff) | 0x0700; // black on white

  if (pos < 0 || pos > 25 * 80)
	panic("pos under/overflow");

  if ((pos / 80) >= 24) { // Scroll up.
	memcpy(CGA_CONSOLE + 80, CGA_CONSOLE, sizeof(CGA_CONSOLE[0]) * 23 * 80);
	pos -= 80;
	memset(CGA_CONSOLE + pos, 0, sizeof(CGA_CONSOLE[0]) * (24 * 80 - pos));
  }

  outb(CRTPORT, 14);
  outb(CRTPORT + 1, pos >> 8);
  outb(CRTPORT, 15);
  outb(CRTPORT + 1, pos);
  CGA_CONSOLE[pos] = ' ' | 0x0700;
}

void cga_console(char *arr) {
  spin_lock(&cga_lock);

  while (*arr) {
	cga_putc(*arr);
	++arr;
  }

  spin_unlock(&cga_lock);
}