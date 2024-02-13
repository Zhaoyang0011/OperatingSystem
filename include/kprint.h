#ifndef _KPRINT_H
#define _KPRINT_H

typedef __builtin_va_list va_list;

#define va_start(ap, np) __builtin_va_start(ap, np)
#define va_end(ap) __builtin_va_end(ap)
#define va_arg(ap, ty) __builtin_va_arg(ap, ty)

void kprint(const char *fmt, ...);

#endif