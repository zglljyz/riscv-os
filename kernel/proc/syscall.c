
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

// copyin - 从用户空间复制数据到内核空间
static int copyin(pagetable_t pagetable, char *dst, uint64_t srcva, uint64_t len) {
    uint64_t n, va0, pa0;

    while(len > 0) {
        va0 = PGROUNDDOWN(srcva);

        // 使用walk查找物理地址
        extern pte_t *walk(pagetable_t, uint64_t, int);
        pte_t *pte = walk(pagetable, va0, 0);
        if(pte == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0)
            return -1;

        pa0 = PTE2PA(*pte);
        n = PGSIZE - (srcva - va0);
        if(n > len)
            n = len;

        // 复制数据
        char *src = (char *)(pa0 + (srcva - va0));
        uint64_t copied = n;  // Save the amount to copy before n is modified
        while(n-- > 0)
            *dst++ = *src++;

        len -= copied;
        srcva += copied;
    }
    return 0;
}

// copyout - 从内核空间复制数据到用户空间
static int copyout(pagetable_t pagetable, uint64_t dstva, char *src, uint64_t len) {
    uint64_t n, va0, pa0;

    while(len > 0) {
        va0 = PGROUNDDOWN(dstva);

        extern pte_t *walk(pagetable_t, uint64_t, int);
        pte_t *pte = walk(pagetable, va0, 0);
        if(pte == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0 || (*pte & PTE_W) == 0)
            return -1;

        pa0 = PTE2PA(*pte);
        n = PGSIZE - (dstva - va0);
        if(n > len)
            n = len;

        char *dst = (char *)(pa0 + (dstva - va0));
        uint64_t copied = n;  // Save before n is modified
        while(n-- > 0)
            *dst++ = *src++;

        len -= copied;
        dstva += copied;
    }
    return 0;
}

uint64_t sys_read(void) {
    struct proc *p = myproc();
    int fd = p->trapframe->a0;
    uint64_t buf_addr = p->trapframe->a1;
    int n = p->trapframe->a2;

    // 参数验证
    if (n < 0 || n > 4096) {
        return -1;
    }

    // 简化实现：只支持标准输入 (fd=0)
    if (fd == 0) {
        extern char uart_getc(void);
        char buf[256];
        int count = 0;

        // 读取最多n个字符（阻塞直到读到换行符或缓冲区满）
        int to_read = n > 256 ? 256 : n;
        for (int i = 0; i < to_read; i++) {
            char c = uart_getc();  // 阻塞等待输入
            buf[count++] = c;
            if (c == '\n' || c == '\r')  // 遇到换行符结束
                break;
        }

        // 复制到用户空间
        if (count > 0) {
            if (copyout(p->pagetable, buf_addr, buf, count) < 0) {
                return -1;
            }
        }

        return count;
    }
    return -1; // 其他 fd 暂不支持
}

uint64_t sys_write(void) {
    struct proc *p = myproc();
    int fd = p->trapframe->a0;
    uint64_t buf_addr = p->trapframe->a1;
    int n = p->trapframe->a2;

    // 参数验证
    if (n < 0 || n > 4096) {
        return -1;
    }

    // 检查无效指针（NULL 或明显无效的地址）
    if (buf_addr == 0) {
        return -1;
    }

    // 简化实现：只支持标准输出 (fd=1)
    if (fd == 1) {
        char buf[256];
        int remaining = n;
        int total_written = 0;

        while (remaining > 0) {
            int chunk = remaining > 256 ? 256 : remaining;

            // 从用户空间复制数据
            if (copyin(p->pagetable, buf, buf_addr, chunk) < 0) {
                return -1;
            }

            // 输出
            for (int i = 0; i < chunk; i++) {
                uart_putc(buf[i]);
            }

            buf_addr += chunk;
            remaining -= chunk;
            total_written += chunk;
        }

        return total_written;
    }
    return -1; // 其他 fd 暂不支持
}

// setpriority 系统调用实现
uint64_t sys_setpriority(void) {
    int pid = myproc()->trapframe->a0;
    int priority = myproc()->trapframe->a1;

    // 验证优先级范围 (0=HIGH, 1=NORMAL, 2=LOW, 3=IDLE)
    if (priority < PRIORITY_HIGH || priority > PRIORITY_IDLE) {
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
    [SYS_read]        = sys_read,
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
