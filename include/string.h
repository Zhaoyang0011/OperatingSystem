#ifndef MEM_H
#define MEM_H

#include <type.h>

KLINE void memset(void *addr, char val, uint32_t size) {
  char *src = (char *)addr;
  for (uint32_t i = 0; i < size; ++i) {
	src[i] = val;
  }
}

KLINE void memcpy(void *s, void *d, uint32_t size) {
  char *src = (char *)s, *dest = (char *)d;
  for (uint32_t i = 0; i < size; ++i) {
	dest[i] = src[i];
  }
}

KLINE int strncmp(const char *p, const char *q, uint32_t n) {
  while (n > 0 && *p && *p == *q)
	n--, p++, q++;
  if (n == 0)
	return 0;
  return (uchar_t)*p - (uchar_t)*q;
}

KLINE int strlen(const char *p) {
  const char *q = p;
  while (*q != 0)
	q++;
  return q - p;
}

#endif