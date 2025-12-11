// 这是第一个用户进程的代码。
// 在没有文件系统的情况下运行。

#include "types.h"
#include "syscall.h"

// --- 内联汇编实现的系统调用包装器 ---
static inline long user_syscall(long num, long arg0, long arg1, long arg2) {
    long ret;
    register long a7 asm("a7") = num;
    register long a0 asm("a0") = arg0;
    register long a1 asm("a1") = arg1;
    register long a2 asm("a2") = arg2;

    asm volatile(
        "ecall"
        : "=r"(a0) // 返回值在 a0 中
        : "r"(a7), "r"(a0), "r"(a1), "r"(a2)
        : "memory"
    );
    ret = a0;
    return ret;
}

// 包装 write 系统调用
void write(int fd, const char *buf, int n) {
    user_syscall(SYS_write, fd, (long)buf, n);
}

// 包装 exit 系统调用
static void exit(int status) {
    user_syscall(SYS_exit, status, 0, 0);
    while(1);
}

// --- 测试程序 ---
void _start() {
    char *msg = "Hello from user space via syscall!\n";
    
    // 调用 write 系统调用
    write(1, msg, 33);

    exit(0);
}
