#pragma once
#include "types.h"

// Legacy SBI Extension Function IDs
#define SBI_SET_TIMER 0
#define SBI_CONSOLE_PUTCHAR 1
#define SBI_CONSOLE_GETCHAR 2
#define SBI_SHUTDOWN 8

// 函数原型
uint64_t sbi_call(uint64_t func_id, uint64_t arg0, uint64_t arg1, uint64_t arg2);

// 便捷的宏封装
#define sbi_set_timer(time) sbi_call(SBI_SET_TIMER, (uint64_t)(time), 0, 0)
#define sbi_console_putchar(ch) sbi_call(SBI_CONSOLE_PUTCHAR, (uint64_t)(ch), 0, 0)
#define sbi_shutdown() sbi_call(SBI_SHUTDOWN, 0, 0, 0)
