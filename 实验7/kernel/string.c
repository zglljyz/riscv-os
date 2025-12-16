
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

void* memmove(void *dst, const void *src, uint64_t n) {
    const char *s = src;
    char *d = dst;

    if (s < d && s + n > d) {
        // 源在目标之前且重叠，从后往前复制
        s += n;
        d += n;
        while (n-- > 0) {
            *--d = *--s;
        }
    } else {
        // 不重叠或源在目标之后，直接从前往后复制
        while (n-- > 0) {
            *d++ = *s++;
        }
    }
    return dst;
}

int strncmp(const char *s1, const char *s2, size_t n) {
    while (n > 0 && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) {
        return 0;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

char* strncpy(char *dst, const char *src, size_t n) {
    char *d = dst;
    while (n > 0 && *src) {
        *d++ = *src++;
        n--;
    }
    while (n > 0) {
        *d++ = '\0';
        n--;
    }
    return dst;
}
