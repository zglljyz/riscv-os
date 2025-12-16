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

// 测试函数声明
void test_round_robin_scheduling(void);
void run_all_tests(void);
void test_aging_starvation_prevention(void);

void test_runner_init(void);

// 第一个进程的包装器
void first_process_wrapper(void) {
    // 调度器持有 ptable_lock，需要先释放
    release(&ptable_lock);

    // 现在可以安全地运行测试
    test_runner_init();
}

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

    // 7. 创建第一个内核线程 "init_test_runner"
    // 这个线程负责运行所有的测试用例
    printf("\nCreating the first kernel thread to run the test suite...\n");
    struct proc *p = allocproc();
    if (p == 0) {
        printf("main: cannot allocate first process\n");
        while(1);
    }

    // 设置第一个线程的入口点和内核栈，使用包装器
    p->context.ra = (uint64_t)first_process_wrapper;
    p->context.sp = p->kstack + PGSIZE;

    // 将其标记为可运行，等待调度器调度
    acquire(&ptable_lock);
    p->state = RUNNABLE;
    release(&ptable_lock);

    // 现在可以安全地启动时钟中断，因为有进程可以处理它们
    start_timer_interrupts();

    // 7. 启动调度器
    printf("Initialization complete. Entering scheduler...\n");
    scheduler();

    // scheduler() 是一个无限循环，代码不应该执行到这里
    printf("Error: scheduler returned!\n");
    while(1);
}


// 它的任务是运行所有的测试，并在结束后暂停系统
void test_runner_init(void) {
    printf("test_runner_init (pid=%d): started.\n", myproc()->pid);
    // 调用所有测试
    run_all_tests();
    // 所有测试完成后，打印信息并挂起
    printf("All tests finished. System halting.\n");
    // 将自己置于 ZOMBIE 状态，但由于没有父进程 wait() 它，
    // 它会一直存在，这可以防止系统继续运行。
    acquire(&ptable_lock);
    exit(0); // exit 会将状态设为 ZOMBIE 并调用 scheduler
             // 因为没有其他 RUNNABLE 进程，系统会空转。
}
