#include "user.h"

// 简单的printf实现
static void putc(char c) {
    write(1, &c, 1);
}

static void puts(const char *s) {
    while (*s) {
        putc(*s++);
    }
}

static void putint(long n, int base) {
    char buf[32];
    int i = 0;
    int neg = 0;

    if (n < 0 && base == 10) {
        neg = 1;
        n = -n;
    }

    if (n == 0) {
        buf[i++] = '0';
    } else {
        while (n > 0) {
            int digit = n % base;
            buf[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
            n /= base;
        }
    }

    if (neg) {
        putc('-');
    }

    while (i > 0) {
        putc(buf[--i]);
    }
}

static void putlonglong(unsigned long long n, int base) {
    char buf[32];
    int i = 0;

    if (n == 0) {
        buf[i++] = '0';
    } else {
        while (n > 0) {
            int digit = n % base;
            buf[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
            n /= base;
        }
    }

    while (i > 0) {
        putc(buf[--i]);
    }
}

// 使用内置的va_list来处理可变参数
typedef __builtin_va_list va_list;
#define va_start(v,l) __builtin_va_start(v,l)
#define va_end(v) __builtin_va_end(v)
#define va_arg(v,l) __builtin_va_arg(v,l)

void printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    while (*fmt) {
        if (*fmt == '%') {
            fmt++;
            // 处理 %llu (long long unsigned)
            if (*fmt == 'l' && *(fmt+1) == 'l' && *(fmt+2) == 'u') {
                unsigned long long val = va_arg(ap, unsigned long long);
                putlonglong(val, 10);
                fmt += 3;
                continue;
            }
            switch (*fmt) {
                case 'd': {
                    int val = va_arg(ap, int);
                    putint(val, 10);
                    break;
                }
                case 'x': {
                    int val = va_arg(ap, int);
                    putint(val, 16);
                    break;
                }
                case 's': {
                    char *s = va_arg(ap, char*);
                    if (s)
                        puts(s);
                    else
                        puts("(null)");
                    break;
                }
                case 'c': {
                    int c = va_arg(ap, int);
                    putc(c);
                    break;
                }
                case '%':
                    putc('%');
                    break;
                default:
                    putc('%');
                    putc(*fmt);
                    break;
            }
            fmt++;
        } else {
            putc(*fmt++);
        }
    }

    va_end(ap);
}
