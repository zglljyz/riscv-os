// User-space shell program
#include "user.h"

#define MAX_CMD_LEN 128

// Type definitions
typedef unsigned long long uint64_t;

// Get CPU cycle counter
static inline uint64_t get_time(void) {
    uint64_t cycles;
    asm volatile("rdcycle %0" : "=r"(cycles));
    return cycles;
}

// 测试函数声明
void test_basic_syscalls(void);
void test_parameter_passing(void);
void test_security(void);
void test_syscall_performance(void);

// 简单的字符串比较
int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

// 简单的strlen
int strlen(const char *s) {
    int n = 0;
    while (s[n])
        n++;
    return n;
}

// 读取一行输入
int readline(char *buf, int max) {
    int i = 0;
    while (i < max - 1) {
        char c;
        if (read(0, &c, 1) != 1)
            break;

        if (c == '\n' || c == '\r') {
            buf[i] = '\0';
            write(1, "\n", 1);
            return i;
        } else if (c == '\b' || c == 127) {  // Backspace
            if (i > 0) {
                i--;
                write(1, "\b \b", 3);  // 退格，空格，退格
            }
        } else {
            buf[i++] = c;
            write(1, &c, 1);  // echo
        }
    }
    buf[i] = '\0';
    return i;
}

// Nice命令 - 测试优先级系统调用
void cmd_nice(char *args) {
    // 解析参数: nice <pid> <priority>
    int pid = 0;
    int priority = 0;

    // 简单的参数解析
    if (args == 0 || args[0] == '\0') {
        printf("Usage: nice <pid> <priority>\n");
        printf("Example: nice 5 2  (set PID 5 to priority 2)\n");
        printf("Priority: 0=HIGH, 1=NORMAL, 2=LOW, 3=IDLE\n");
        return;
    }

    // 解析PID
    char *p = args;
    while (*p == ' ') p++; // 跳过空格
    while (*p >= '0' && *p <= '9') {
        pid = pid * 10 + (*p - '0');
        p++;
    }

    // 解析优先级
    while (*p == ' ') p++; // 跳过空格
    while (*p >= '0' && *p <= '9') {
        priority = priority * 10 + (*p - '0');
        p++;
    }

    if (pid == 0) {
        printf("Invalid PID\n");
        return;
    }

    if (priority < 0 || priority > 3) {
        printf("Invalid priority. Must be 0-3\n");
        return;
    }

    printf("Setting PID %d priority to %d...\n", pid, priority);

    int result = setpriority(pid, priority);

    if (result == 0) {
        int current = getpriority(pid);
        printf("Success! PID %d priority is now %d\n", pid, current);
    } else {
        printf("Failed to change priority (process may not exist)\n");
    }
}

// 基础功能测试
void test_basic_syscalls(void) {
    printf("\n=== Testing Basic System Calls ===\n");

    // 测试getpid
    int pid = getpid();
    printf("Current PID: %d\n", pid);

    // 只有进程 1 进行 fork 测试，避免 fork bomb
    if (pid == 1) {
        // 测试fork
        printf("Testing fork()...\n");

        // 使用临时变量立即保存fork返回值
        int fork_ret = fork();

        // 立即打印，避免被覆盖
        printf("IMMEDIATE: fork returned %d\n", fork_ret);

        int child_pid = fork_ret;  // 保存到另一个变量

        // DEBUG: 显示每个进程看到的fork返回值
        int current_pid = getpid();
        printf("DEBUG: PID %d sees fork() returned %d\n", current_pid, child_pid);

        if (child_pid == 0) {
            // 子进程
            printf("Child process: PID=%d, Parent PID=%d\n", getpid(), pid);
            printf("Child exiting with status 42\n");
            exit(42);
        } else if (child_pid > 0) {
            // 父进程
            printf("Parent process: Created child with PID=%d\n", child_pid);
            int status;
            printf("Parent waiting for child...\n");
            wait(&status);
            printf("Child exited with status: %d\n", status);
        }
    } else {
        printf("Skipping fork test (not PID 1)\n");
    }
}

// 参数传递测试
void test_parameter_passing(void) {
    printf("\n=== Testing Parameter Passing ===\n");

    char buffer[] = "Hello from test!\n";

    // 测试write
    printf("Testing write()...\n");
    int bytes_written = write(1, buffer, strlen(buffer));
    printf("Wrote %d bytes\n", bytes_written);

    // 测试边界情况
    printf("Testing boundary cases...\n");

    // 无效文件描述符
    int result = write(-1, buffer, 10);
    printf("Invalid fd write result: %d (expected < 0)\n", result);

    // 空指针 (这可能导致错误)
    result = write(1, (void*)0, 10);
    printf("NULL pointer write result: %d (expected < 0)\n", result);

    // 负数长度
    result = write(1, buffer, -1);
    printf("Negative length write result: %d (expected < 0)\n", result);
}

// 安全性测试
void test_security(void) {
    printf("\n=== Testing Security ===\n");

    // 测试无效指针访问
    printf("Testing invalid pointer access...\n");
    char *invalid_ptr = (char*)0x1000000;  // 可能无效的地址
    int result = write(1, invalid_ptr, 10);
    printf("Invalid pointer write result: %d (expected < 0)\n", result);

    // 测试边界情况 - 只测试write，read会阻塞
    printf("Testing buffer boundary (write only)...\n");
    char small_buf[4]="1\n";
    result = write(1, small_buf, strlen(small_buf));
    printf("Write to small buffer result: %d bytes\n", result);
}

// 性能测试
void test_syscall_performance(void) {
    printf("\n=== Testing System Call Performance ===\n");

    uint64_t start_time = get_time();

    // 大量系统调用测试
    for (int i = 0; i < 10000; i++) {
        getpid(); // 简单的系统调用
    }

    uint64_t end_time = get_time();
    printf("10000 getpid() calls took %llu cycles\n", end_time - start_time);
    printf("Average: %llu cycles per call\n", (end_time - start_time) / 10000);

}

// Test命令 - 运行所有测试
void cmd_test(void) {
    printf("\n========================================\n");
    printf("  System Call Test Suite\n");
    printf("========================================\n");

    test_basic_syscalls();
    test_parameter_passing();
    test_security();
    test_syscall_performance();

    printf("\n========================================\n");
    printf("  All Tests Completed\n");
    printf("========================================\n\n");
}

// 主shell循环
int main(void) {
    char cmd[MAX_CMD_LEN];

    printf("\n");
    printf("=====================================\n");
    printf("  RISC-V OS Shell\n");
    printf("=====================================\n");
    printf("Available commands:\n");
    printf("  nice  - Test priority system calls\n");
    printf("  test  - Run system call test suite\n");
    printf("  help  - Show this help message\n");
    printf("  exit  - Exit shell\n");
    printf("=====================================\n\n");

    while (1) {
        printf("shell> ");

        int len = readline(cmd, MAX_CMD_LEN);

        if (len == 0) {
            continue;
        }

        // 解析命令和参数
        char *command = cmd;
        char *args = 0;

        // 查找第一个空格，分离命令和参数
        for (int i = 0; i < len; i++) {
            if (cmd[i] == ' ') {
                cmd[i] = '\0';
                args = &cmd[i + 1];
                break;
            }
        }

        if (strcmp(command, "nice") == 0) {
            cmd_nice(args);
        } else if (strcmp(command, "test") == 0) {
            cmd_test();
        } else if (strcmp(command, "help") == 0) {
            printf("Available commands:\n");
            printf("  nice <pid> <priority> - Set process priority\n");
            printf("  test  - Run system call test suite\n");
            printf("  help  - Show this help message\n");
            printf("  exit  - Exit shell\n");
        } else if (strcmp(command, "exit") == 0) {
            printf("Goodbye!\n");
            while(1){}
        } else {
            printf("Unknown command: %s\n", command);
            printf("Type 'help' for available commands\n");
        }
    }

    exit(0);
}
