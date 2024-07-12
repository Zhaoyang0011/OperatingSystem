#include "cga.h"
#include <kprint.h>
#include <type.h>

KLINE char *kstrcpy(char *s, char *d) {
  while (*s) {
    *d = *s;
    ++s;
    ++d;
  }
  return s;
}

KLINE char *kprint_int(char *str, int n, int base) {
  register char *p;
  char strbuf[36];
  p = &strbuf[35];
  *p = 0;

  do {
    *--p = "0123456789abcdef"[n % base];
    n /= base;
  } while (n);

  while (*p) {
    *str++ = *p++;
  }

  return str;
}

void kprint(const char *fmt, ...) {
  char buf[512];
  va_list args;
  va_start(args, fmt);
  char *p = buf;
  while (*fmt && p < buf + 510) {
    if (*fmt != '%') {
      *p++ = *fmt++;
      continue;
    }
    fmt++;
    switch (*fmt) {
      case 'd':p = kprint_int(p, va_arg(args, uint_t), 10);
        fmt++;
        break;
      case 'x':p = kprint_int(p, va_arg(args, uint_t), 16);
        fmt++;
        break;
      case 's':p = kstrcpy(p, (char *)va_arg(args, uint_t));
        fmt++;
        break;
      default:break;
    }
  }
  va_end(args);
  cga_console(buf);
}