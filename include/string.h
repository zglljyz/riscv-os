
#pragma once
#include "types.h"

void* memset(void *p, int c, size_t n);
void* memcpy(void *dst, const void *src, uint64_t n);
void* memmove(void *dst, const void *src, uint64_t n);
int strncmp(const char *s1, const char *s2, size_t n);
char* strncpy(char *dst, const char *src, size_t n); 
