
#include "cpu.h"
#include "types.h"
#include "riscv.h"
#include "proc.h"
#include "syscall.h"

// 声明外部函数和变量
extern int fork(void);
extern void exit(int);
extern int wait(uint64_t);
extern int getpid(void);
extern void yield(void);
extern struct proc procs[NPROC];   // 添加进程数组的外部声明
extern struct spinlock ptable_lock;  // 添加进程表锁的外部声明
void printf(const char *fmt, ...);
extern void uart_putc(char);

uint64_t sys_write(void) {
    struct proc *p = myproc();
    int fd = p->trapframe->a0;
    uint64_t buf_addr = p->trapframe->a1;
    int n = p->trapframe->a2;

    // 简化实现：只支持标准输出 (fd=1)
    if (fd == 1) {
        char *buf = (char *)buf_addr;
        for (int i = 0; i < n; i++) {
            uart_putc(buf[i]);
        }
        return n; // 返回成功写入的字节数
    }
    return -1; // 其他 fd 暂不支持
}

// setpriority 系统调用实现
uint64_t sys_setpriority(void) {
    int pid = myproc()->trapframe->a0;
    int priority = myproc()->trapframe->a1;

    // 验证优先级范围
    if (priority < PRIORITY_HIGH || priority > PRIORITY_LOW) {
        return -1; // 无效的优先级
    }

    acquire(&ptable_lock);
    for (struct proc *p = procs; p < &procs[NPROC]; p++) {
        if (p->pid == pid) {
            if (p->state == UNUSED) {
                release(&ptable_lock);
                return -1; // 进程不存在
            }
            set_priority(p, priority);
            release(&ptable_lock);
            printf("setpriority: set process %d priority to %d\n", pid, priority);
            return 0; // 成功
        }
    }
    release(&ptable_lock);
    return -1; // 进程不存在
}

// getpriority 系统调用实现
uint64_t sys_getpriority(void) {
    int pid = myproc()->trapframe->a0;

    acquire(&ptable_lock);
    for (struct proc *p = procs; p < &procs[NPROC]; p++) {
        if (p->pid == pid) {
            if (p->state == UNUSED) {
                release(&ptable_lock);
                return -1; // 进程不存在
            }
            int priority = p->priority;
            release(&ptable_lock);
            return priority;
        }
    }
    release(&ptable_lock);
    return -1; // 进程不存在
}

// 系统调用函数指针数组
static uint64_t (*syscalls[])(void) = {
    [SYS_fork]        = (uint64_t (*)())fork,
    [SYS_exit]        = (uint64_t (*)())exit,
    [SYS_wait]        = (uint64_t (*)())wait,
    [SYS_getpid]      = (uint64_t (*)())getpid,
    [SYS_yield]       = (uint64_t (*)())yield,
    [SYS_write]       = sys_write,
    [SYS_setpriority] = sys_setpriority,
    [SYS_getpriority] = sys_getpriority,
};

// 从 a0-a5 寄存器获取第 n 个参数
int argint(int n, int *ip) {
    struct proc *p = myproc();
    switch (n) {
        case 0: *ip = p->trapframe->a0; break;
        case 1: *ip = p->trapframe->a1; break;
        // ...
        default: return -1;
    }
    return 0;
}

void syscall(void) {
    int num;
    struct proc *p = myproc();

    num = p->trapframe->a7; // a7 存放系统调用号
    if (num > 0 && num < (sizeof(syscalls)/sizeof(syscalls[0])) && syscalls[num]) {
        // 调用相应的系统调用函数
        p->trapframe->a0 = syscalls[num]();
    } else {
        printf("unknown syscall %d\n", num);
        p->trapframe->a0 = -1;
    }
}

int getpid(void) {
    return myproc()->pid;
}
