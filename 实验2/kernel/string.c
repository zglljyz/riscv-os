
// 包含一些常用的内存操作函数。


#include "types.h"
#include "string.h"


void *memset(void *s, int c, size_t n)
{
    unsigned char *p = (unsigned char *)s;
    while (n > 0) {
        *p++ = (unsigned char)c;
        n--;
    }
    return s;
}


void* memcpy(void *dst, const void *src, size_t n)
{
    char *d = (char*)dst;
    const char *s = (const char*)src;
    while (n > 0) {
        *d++ = *s++;
        n--;
    }
    return dst;
}
