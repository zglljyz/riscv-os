
#pragma once

// 系统调用编号
#define SYS_fork    1
#define SYS_exit    2
#define SYS_wait    3
#define SYS_getpid  4
#define SYS_yield   5
#define SYS_read    6
#define SYS_write   7
#define SYS_setpriority  8
#define SYS_getpriority  9

void syscall(void);
