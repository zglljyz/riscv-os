#include "cpu.h"
#include "proc.h"
#include "vm.h"
#include "types.h"
#include "riscv.h"
#include "trap.h"

// --- 外部函数声明 ---
void printf(const char*, ...);
void pmm_init();
void kvminit();
void kvminithart();
void trap_init_s();
void virtio_disk_init();  // 添加 virtio-blk 初始化

// 从 proc.c 中引入的初始化函数
void cpu_init(void);
void proc_init(void);

void scheduler(void) __attribute__((noreturn));
struct proc* allocproc(void);

// 用户进程初始化
void userinit(void);

void main(void) {
    printf("Hello, RISC-V OS! In S-mode.\n");

    // 1. 初始化物理内存管理器
    pmm_init();
    printf("PMM Initialized.\n");

    // 2. 初始化内核页表并开启分页
    kvminit();
    printf("Kernel Page Table Initialized.\n");
    kvminithart();
    printf("Paging enabled!\n");

    // 3.初始化CPU结构体
    cpu_init();
    printf("CPU structures initialized.\n");

    // 4. 初始化进程管理
    proc_init();
    printf("Process table initialized.\n");

    // 5. 初始化中断处理
    trap_init_s();
    printf("Supervisor traps initialized.\n");

    // 6. 初始化 virtio-blk 设备
    virtio_disk_init();
    printf("VirtIO block device initialized.\n");

    // 7. 创建第一个用户进程运行系统调用测试
    printf("\nCreating first user process to run system call tests...\n");
    userinit();
    printf("First user process created successfully\n");

    // 现在可以安全地启动时钟中断
    start_timer_interrupts();

    // 8. 启动调度器
    printf("Initialization complete. Entering scheduler...\n");
    scheduler();

    // scheduler() 是一个无限循环,代码不应该执行到这里
    printf("Error: scheduler returned!\n");
    while(1);
}
