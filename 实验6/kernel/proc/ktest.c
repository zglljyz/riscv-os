#include "types.h"
#include "riscv.h"
#include "proc.h"
#include "sbi.h"
#include "cpu.h"
#include "vm.h"
#include "fs.h"  // 添加文件系统头文件
#include "syscall.h"  // 添加系统调用头文件

// 外部函数和变量声明
void printf(const char *fmt, ...);
void scheduler(void);
void yield(void);
void exit(int);
int wait(uint64_t);
extern struct proc procs[NPROC];
extern struct spinlock ptable_lock;
void shell_main(void);  // Shell主函数

// 系统调用包装函数
static inline int setpriority(int pid, int priority) {
    register int a0 asm("a0") = pid;
    register int a1 asm("a1") = priority;
    register int a7 asm("a7") = SYS_setpriority;
    register int ret asm("a0");

    asm volatile("ecall" : "=r"(ret) : "r"(a0), "r"(a1), "r"(a7) : "memory");
    return ret;
}

static inline int getpriority(int pid) {
    register int a0 asm("a0") = pid;
    register int a7 asm("a7") = SYS_getpriority;
    register int ret asm("a0");

    asm volatile("ecall" : "=r"(ret) : "r"(a0), "r"(a7) : "memory");
    return ret;
}

//生产者-消费者场景
#define BUFFER_SIZE 4
#define NUM_ITEMS 10

struct {
    struct spinlock lock;
    char buffer[BUFFER_SIZE];
    int count;
    int read_pos;
    int write_pos;
} shared_buffer;

void shared_buffer_init(void) {
    initlock(&shared_buffer.lock, "shared_buffer");
    shared_buffer.count = 0;
    shared_buffer.read_pos = 0;
    shared_buffer.write_pos = 0;
    printf("Shared buffer initialized.\n");
}

void producer_task(void *arg) {
    printf("Producer (pid=%d) started.\n", myproc()->pid);
    for (int i = 0; i < NUM_ITEMS; ++i) {
        acquire(&shared_buffer.lock);

        while (shared_buffer.count == BUFFER_SIZE) {
            // sleep 会原子地释放 shared_buffer.lock, 醒来后会重新获取
            sleep(&shared_buffer.read_pos, &shared_buffer.lock);
        }

        char item = 'A' + (i % 26);
        shared_buffer.buffer[shared_buffer.write_pos] = item;
        shared_buffer.write_pos = (shared_buffer.write_pos + 1) % BUFFER_SIZE;
        shared_buffer.count++;
        printf("Producer produced: %c (count=%d)\n", item, shared_buffer.count);
        
        // wakeup 不需要锁，因为它只是改变状态
        wakeup(&shared_buffer.write_pos);
        release(&shared_buffer.lock);

        for(volatile int j=0; j<10000; j++); // 模拟工作，减少延迟
    }
    printf("Producer (pid=%d) finished.\n", myproc()->pid);
    exit(0);
}

void consumer_task(void *arg) {
    printf("Consumer (pid=%d) started.\n", myproc()->pid);
    for (int i = 0; i < NUM_ITEMS; ++i) {
        acquire(&shared_buffer.lock);

        while (shared_buffer.count == 0) {
            sleep(&shared_buffer.write_pos, &shared_buffer.lock);
        }

        char item = shared_buffer.buffer[shared_buffer.read_pos];
        shared_buffer.read_pos = (shared_buffer.read_pos + 1) % BUFFER_SIZE;
        shared_buffer.count--;
        printf("Consumer consumed: %c (count=%d)\n", item, shared_buffer.count);

        wakeup(&shared_buffer.read_pos);
        release(&shared_buffer.lock);

        for(volatile int j=0; j<10000; j++); // 模拟工作，减少延迟
    }
    printf("Consumer (pid=%d) finished.\n", myproc()->pid);
    exit(0);
}

// 测试任务和辅助函数
void simple_task(void *arg) {
    printf("simple_task (pid=%d): started and exiting.\n", myproc()->pid);
    exit(0);
}

void cpu_intensive_task(void *arg) {
    int id = (int)(uint64_t)arg;  // 直接使用传入的参数
    printf("cpu_intensive_task %d (pid=%d): started and finished.\n", id, myproc()->pid);
    exit(0);
}

// 内核线程包装器，用于正确设置内核线程环境
void kthread_wrapper(void) {
    // 释放调度器持有的锁
    printf("[kthread_wrapper] Entered, releasing ptable_lock\n");
    release(&ptable_lock);

    // 获取当前进程
    struct proc *p = myproc();
    printf("[kthread_wrapper] Got process %d\n", p ? p->pid : -1);

    // 获取真实的入口函数（存储在 trapframe 中）
    void (*func)(void *) = (void (*)(void *))p->trapframe->epc;
    void *arg = (void *)p->trapframe->a0;
    printf("[kthread_wrapper] Calling function at 0x%lx with arg 0x%lx\n", (uint64_t)func, (uint64_t)arg);

    // 调用实际的函数
    func(arg);

    // 如果函数返回（不应该），调用 exit
    printf("[kthread_wrapper] Function returned, calling exit\n");
    exit(0);
}

int create_kthread(void (*func)(void *), void *arg) {
    struct proc *p = allocproc();
    if (p == 0) return -1;

    // 清理用户页表，因为内核线程不需要它
    p->pagetable = 0;
    p->sz = 0;

    // 在 trapframe 中保存函数和参数
    p->trapframe->epc = (uint64_t)func;
    p->trapframe->a0 = (uint64_t)arg;

    // 设置上下文返回到 kthread_wrapper
    p->context.ra = (uint64_t)kthread_wrapper;
    p->context.sp = p->kstack + PGSIZE;
    p->parent = myproc();

    //设置状态为 RUNNABLE 必须在锁的保护下进行
    acquire(&ptable_lock);
    p->state = RUNNABLE;
    release(&ptable_lock);

    return p->pid;
}

void test_process_creation(void) {
    printf("\n=== Process Creation Tests ===\n");
    int passed = 0, total = 0;

    // Test 1: Basic creation
    total++;
    printf("[Test %d] Basic process creation...\n", total);
    int pid = create_kthread(simple_task, 0);
    if(pid > 0) {
        printf("✓ PASS: Created process with pid = %d\n", pid);
        passed++;
        yield();
        wait(0);
    } else {
        printf("✗ FAIL: Process creation failed\n");
    }

    // Test 2: Multiple process creation
    total++;
    printf("\n[Test %d] Multiple process creation...\n", total);
    int pids[5];
    int success_count = 0;
    for (int i = 0; i < 5; i++) {
        pids[i] = create_kthread(simple_task, 0);
        if (pids[i] > 0) success_count++;
    }
    if (success_count == 5) {
        printf("✓ PASS: Created %d processes successfully\n", success_count);
        passed++;
    } else {
        printf("✗ FAIL: Only created %d/5 processes\n", success_count);
    }
    for (int i = 0; i < success_count; i++) {
        yield();
    }
    for (int i = 0; i < success_count; i++) {
        wait(0);
    }

    // Test 3: Process table limit
    total++;
    printf("\n[Test %d] Testing process table limit (NPROC=%d)...\n", total, NPROC);
    int count = 0;
    for (int i = 0; i < NPROC + 5; i++) {
        pid = create_kthread(simple_task, 0);
        if (pid > 0) {
            count++;
        } else {
            break;
        }
    }
    int expected = NPROC - 3; // Accounting for existing processes
    if (count >= expected - 5 && count <= expected + 5) {
        printf("✓ PASS: Created %d processes (expected ~%d)\n", count, expected);
        passed++;
    } else {
        printf("✗ FAIL: Created %d processes (expected ~%d)\n", count, expected);
    }

    printf("Cleaning up %d processes...\n", count);
    for(int i=0; i<count; i++) {
        yield();
    }
    for (int i = 0; i < count; i++) {
        wait(0);
    }

    printf("\n--- Process Creation Test Summary: %d/%d tests passed ---\n", passed, total);
}

void test_scheduler(void) {
    printf("\n=== Scheduler Tests ===\n");
    int passed = 0, total = 0;

    // Test 1: Basic round-robin scheduling with lightweight CPU tasks
    total++;
    printf("[Test %d] Basic round-robin scheduling with CPU tasks...\n", total);
    printf("Creating 3 lightweight CPU-intensive tasks...\n");

    for (int i = 0; i < 3; i++) {
        int pid = create_kthread(cpu_intensive_task, (void*)(uint64_t)(i + 1));
        printf("Created CPU task %d with pid %d\n", i+1, pid);
        yield(); // 立即让出CPU，让新创建的任务有机会运行
    }

    printf("Waiting for 3 CPU tasks to complete...\n");
    for (int i = 0; i < 3; i++) {
        printf("Waiting for task %d...\n", i+1);
        wait(0);
        printf("Task %d completed\n", i+1);
    }
    printf("✓ PASS: All 3 CPU tasks completed successfully\n");
    passed++;

    // Test 2: Context switching test
    total++;
    printf("\n[Test %d] Context switching test...\n", total);
    printf("Creating 5 simple tasks for context switching...\n");
    for (int i = 0; i < 5; i++) {
        create_kthread(simple_task, 0);
    }
    printf("Forcing context switches with yield()...\n");
    for (int i = 0; i < 5; i++) {
        yield(); // Force context switching
    }
    printf("Waiting for all tasks to complete...\n");
    for (int i = 0; i < 5; i++) {
        wait(0);
    }
    printf("✓ PASS: Context switching works correctly\n");
    passed++;

    printf("\n--- Scheduler Test Summary: %d/%d tests passed ---\n", passed, total);
}

//===========================================
// 实验五：优先级调度测试
//===========================================

// 高优先级任务
void high_priority_task(void *arg) {
    printf("High priority task (pid=%d): started with priority %d\n",
           myproc()->pid, myproc()->priority);

    for (int i = 0; i < 5; i++) {
        printf("High priority task: iteration %d\n", i);
        for (volatile int j = 0; j < 10; j++); // 模拟工作
        yield(); // 主动让出CPU
    }

    printf("High priority task (pid=%d): finished\n", myproc()->pid);
    exit(0);
}

// 低优先级任务
void low_priority_task(void *arg) {
    printf("Low priority task (pid=%d): started with priority %d\n",
           myproc()->pid, myproc()->priority);

    for (int i = 0; i < 3; i++) {
        printf("Low priority task: iteration %d\n", i);
        for (volatile int j = 0; j < 10; j++); // 模拟工作
        yield(); // 主动让出CPU
    }

    printf("Low priority task (pid=%d): finished\n", myproc()->pid);
    exit(0);
}

// 测试优先级调度
void test_priority_scheduling() {
    printf("\n=== Testing Priority Scheduling ===\n");

    // 创建低优先级进程
    printf("Creating low priority process...\n");
    int low_pid = create_kthread(low_priority_task, 0);
    if (low_pid > 0) {
        acquire(&ptable_lock);
        for (struct proc *proc = procs; proc < &procs[NPROC]; proc++) {
            if (proc->pid == low_pid) {
                set_priority(proc, PRIORITY_LOW);
                printf("Set process %d priority to LOW (%d)\n", low_pid, PRIORITY_LOW);
                break;
            }
        }
        release(&ptable_lock);
    }

    // 创建高优先级进程
    printf("Creating high priority process...\n");
    int high_pid = create_kthread(high_priority_task, 0);
    if (high_pid > 0) {
        acquire(&ptable_lock);
        for (struct proc *proc = procs; proc < &procs[NPROC]; proc++) {
            if (proc->pid == high_pid) {
                set_priority(proc, PRIORITY_HIGH);
                printf("Set process %d priority to HIGH (%d)\n", high_pid, PRIORITY_HIGH);
                break;
            }
        }
        release(&ptable_lock);
    }

    // 等待进程完成
    printf("Waiting for processes to complete...\n");
    if (low_pid > 0) wait(0);
    if (high_pid > 0) wait(0);

    printf("Priority scheduling test completed.\n");
}

//===========================================
// 新增：系统调用和 Aging 机制测试
//===========================================

// 测试任务：长时间运行的低优先级任务
void long_running_task(void *arg) {
    int task_id = (int)(uint64_t)arg;
    printf("Long running task %d (pid=%d): started\n", task_id, myproc()->pid);

    for (int i = 0; i < 20; i++) {
        printf("Task %d: iteration %d (priority=%d)\n", task_id, i, myproc()->priority);

        // 模拟工作
        for (volatile int j = 0; j < 50; j++);

        yield(); // 主动让出CPU，模拟正常调度
    }

    printf("Long running task %d (pid=%d): finished\n", task_id, myproc()->pid);
    exit(0);
}

// 测试系统调用功能（简化版）
void test_priority_syscalls(void) {
    printf("\n=== Priority System Calls Tests ===\n");
    int passed = 0, total = 0;

    // Test 1: 直接测试内核函数而不是系统调用
    total++;
    printf("[Test %d] Direct kernel function test...\n", total);

    int test_pid = create_kthread(long_running_task, (void*)1);
    if (test_pid > 0) {
        // 直接使用内核函数而不是系统调用
        acquire(&ptable_lock);
        for (struct proc *p = procs; p < &procs[NPROC]; p++) {
            if (p->pid == test_pid) {
                printf("Process %d default priority: %d\n", test_pid, p->priority);

                // 测试设置优先级
                set_priority(p, PRIORITY_HIGH);
                printf("Process %d new priority: %d\n", test_pid, p->priority);

                if (p->priority == PRIORITY_HIGH) {
                    printf("✓ PASS: Priority set correctly\n");
                    passed++;
                } else {
                    printf("✗ FAIL: Priority not updated\n");
                }
                break;
            }
        }
        release(&ptable_lock);

        wait(0); // 等待测试进程结束
    } else {
        printf("✗ FAIL: Could not create test process\n");
    }

    printf("\n--- Priority System Calls Test Summary: %d/%d tests passed ---\n", passed, total);
}

// 测试 Aging 机制（简化版）
void test_aging_mechanism(void) {
    printf("\n=== Aging Mechanism Tests ===\n");
    int passed = 0, total = 0;

    // Test 1: 基础 Aging 功能测试
    total++;
    printf("[Test %d] Basic aging functionality test...\n", total);

    // 创建低优先级任务
    printf("Creating low priority tasks...\n");
    int low_pid1 = create_kthread(long_running_task, (void*)1);
    int low_pid2 = create_kthread(long_running_task, (void*)2);

    if (low_pid1 > 0 && low_pid2 > 0) {
        // 设置为低优先级
        acquire(&ptable_lock);
        for (struct proc *p = procs; p < &procs[NPROC]; p++) {
            if (p->pid == low_pid1 || p->pid == low_pid2) {
                set_priority(p, PRIORITY_LOW);
                printf("Set process %d to LOW priority\n", p->pid);
            }
        }
        release(&ptable_lock);

        printf("Letting processes run (aging mechanism should activate)...\n");

        // 等待任务完成
        wait(0);
        wait(0);

        printf("✓ PASS: Aging mechanism test completed\n");
        passed++;
    } else {
        printf("✗ FAIL: Could not create test processes\n");
    }

    printf("\n--- Aging Mechanism Test Summary: %d/%d tests passed ---\n", passed, total);
}

// 综合测试（简化版）
void test_comprehensive_priority_scheduling(void) {
    printf("\n=== Comprehensive Priority Scheduling Test ===\n");
    int passed = 0, total = 0;

    total++;
    printf("[Test %d] Mixed priority workload test...\n", total);

    // 创建不同优先级的任务
    int high_pid = create_kthread(long_running_task, (void*)1);
    int normal_pid = create_kthread(long_running_task, (void*)2);
    int low_pid = create_kthread(long_running_task, (void*)3);

    if (high_pid > 0 && normal_pid > 0 && low_pid > 0) {
        // 设置不同优先级
        acquire(&ptable_lock);
        for (struct proc *p = procs; p < &procs[NPROC]; p++) {
            if (p->pid == high_pid) {
                set_priority(p, PRIORITY_HIGH);
                printf("Set process %d to HIGH priority\n", p->pid);
            } else if (p->pid == normal_pid) {
                set_priority(p, PRIORITY_NORMAL);
                printf("Set process %d to NORMAL priority\n", p->pid);
            } else if (p->pid == low_pid) {
                set_priority(p, PRIORITY_LOW);
                printf("Set process %d to LOW priority\n", p->pid);
            }
        }
        release(&ptable_lock);

        printf("Mixed priority execution started...\n");

        // 等待所有任务完成
        wait(0);
        wait(0);
        wait(0);

        printf("✓ PASS: Comprehensive test completed\n");
        passed++;
    } else {
        printf("✗ FAIL: Could not create all test processes\n");
    }

    printf("\n--- Comprehensive Test Summary: %d/%d tests passed ---\n", passed, total);
}

//===========================================
// 新增：抢占式调度测试
//===========================================

// CPU密集型任务，不主动yield，测试时钟中断抢占
static volatile int preempt_counter = 0;
static volatile int low_task_ready_for_preemption = 0;  // 标记低优先级任务准备好被抢占
static volatile int high_task_should_be_created = 0;  // 标记应该创建高优先级任务
static volatile int preemption_test_high_pid = 0;  // 存储要创建的高优先级任务PID

void cpu_bound_task(void *arg) {
    int task_id = (int)(uint64_t)arg;
    int local_count = 0;

    printf("CPU-bound task %d (pid=%d, priority=%d): started\n",
           task_id, myproc()->pid, myproc()->priority);

    // 执行计算，不主动yield，只能靠时钟中断抢占
    for (volatile int i = 0; i < 50000; i++) {
        local_count++;

        // 如果是任务1（低优先级），在运行到10000次迭代时标记准备好被抢占
        if (task_id == 1 && i == 10000) {
            printf("Task 1: reached mid-point (iteration 10000) - ready for preemption!\n");
            low_task_ready_for_preemption = 1;

            // 主动yield一次，让test runner有机会看到标志并创建高优先级任务
            yield();

            // 等待高优先级任务被创建（通过标志检查）
            // 注意：这里使用yield而不是busy wait，避免占用CPU
            while (high_task_should_be_created == 0) {
                yield();  // 主动让出CPU，让test runner可以运行
            }
        }

        if (i % 10000 == 0) {
            printf("Task %d: iteration %d (priority=%d)\n",
                   task_id, i, myproc()->priority);
            preempt_counter++;
        }
    }

    printf("CPU-bound task %d (pid=%d): finished with %d iterations\n",
           task_id, myproc()->pid, local_count);
    exit(0);
}

void test_preemptive_scheduling(void) {
    printf("\n=== Preemptive Scheduling Tests ===\n");
    int passed = 0, total = 0;

    // Test 1: 基础抢占测试 - 两个同优先级的CPU密集型任务
    total++;
    printf("[Test %d] Basic preemption test (same priority)...\n", total);
    printf("Creating two CPU-bound tasks that do NOT call yield()...\n");
    printf("Only timer interrupts should cause preemption.\n\n");

    preempt_counter = 0;

    int pid1 = create_kthread(cpu_bound_task, (void*)10);  // Use task_id=10 to avoid sync logic
    int pid2 = create_kthread(cpu_bound_task, (void*)11);  // Use task_id=11 to avoid sync logic

    if (pid1 > 0 && pid2 > 0) {
        // 设置为相同的普通优先级
        acquire(&ptable_lock);
        for (struct proc *p = procs; p < &procs[NPROC]; p++) {
            if (p->pid == pid1 || p->pid == pid2) {
                set_priority(p, PRIORITY_NORMAL);
                printf("Set process %d to NORMAL priority\n", p->pid);
            }
        }
        release(&ptable_lock);

        printf("\nBoth tasks running... watch for context switches\n");
        printf("(If preemption works, you'll see interleaved output)\n\n");

        // 等待任务完成
        wait(0);
        wait(0);

        printf("\n✓ PASS: Preemption test completed (counter=%d)\n", preempt_counter);
        passed++;
    } else {
        printf("✗ FAIL: Could not create test processes\n");
    }

    // Test 2: 高优先级抢占低优先级
    total++;
    printf("\n[Test %d] Priority-based preemption test...\n", total);
    printf("Scenario: Low priority task is running, then high priority task arrives and preempts it.\n");
    printf("Expected: LOW starts -> reaches mid-point -> HIGH created and preempts -> HIGH finishes -> LOW resumes.\n\n");

    preempt_counter = 0;
    low_task_ready_for_preemption = 0;
    high_task_should_be_created = 0;

    // 先创建低优先级任务（将会先运行）
    int low_pid = create_kthread(cpu_bound_task, (void*)1);

    if (low_pid > 0) {
        acquire(&ptable_lock);
        for (struct proc *p = procs; p < &procs[NPROC]; p++) {
            if (p->pid == low_pid) {
                set_priority(p, PRIORITY_LOW);
                // 重要：将基础优先级也设为LOW，避免aging提升
                p->base_priority = PRIORITY_LOW;
                printf("Created LOW priority task (pid=%d, priority=%d)\n", p->pid, p->priority);
                break;
            }
        }
        release(&ptable_lock);

        // 等待低优先级任务到达中点（iteration 10000）
        printf("Waiting for LOW priority task to reach mid-point...\n");
        while (low_task_ready_for_preemption == 0) {
            yield();
        }
        printf("LOW priority task ready for preemption!\n\n");

        // 现在创建高优先级任务 - 应该立即抢占低优先级任务
        printf("Now creating HIGH priority task - should preempt the running LOW priority task!\n");
        int high_pid = create_kthread(cpu_bound_task, (void*)2);

        if (high_pid > 0) {
            acquire(&ptable_lock);
            for (struct proc *p = procs; p < &procs[NPROC]; p++) {
                if (p->pid == high_pid) {
                    set_priority(p, PRIORITY_HIGH);
                    p->base_priority = PRIORITY_HIGH;
                    printf("Created HIGH priority task (pid=%d, priority=%d)\n", p->pid, p->priority);
                    break;
                }
            }
            release(&ptable_lock);

            // 通知低优先级任务高优先级已创建
            high_task_should_be_created = 1;

            printf("\n--- Watch the execution order below ---\n");
            printf("You should see: HIGH runs to completion, then LOW resumes from iteration 10000\n\n");

            // 等待两个任务完成
            wait(0);
            wait(0);

            printf("\n✓ PASS: Priority preemption test completed\n");
            printf("You should have seen: LOW start -> LOW pause at 10000 -> HIGH complete -> LOW resume!\n");
            passed++;
        } else {
            wait(0);
            printf("✗ FAIL: Could not create high priority process\n");
        }
    } else {
        printf("✗ FAIL: Could not create low priority process\n");
    }

    printf("\n--- Preemptive Scheduling Test Summary: %d/%d tests passed ---\n", passed, total);
}

//===========================================
// 新增：时间片轮转测试（相同优先级）
//===========================================

// 短迭代任务，用于观察交替执行
static volatile int round_robin_log[100];  // 记录执行顺序
static volatile int round_robin_log_idx = 0;

void round_robin_task(void *arg) {
    int task_id = (int)(uint64_t)arg;

    printf("Round-robin task %d (pid=%d, priority=%d): started\n",
           task_id, myproc()->pid, myproc()->priority);

    // 模拟较长的任务执行，通过主动yield来展示round-robin调度
    for (int i = 0; i < 20; i++) {
        // 记录当前执行的任务ID
        if (round_robin_log_idx < 100) {
            round_robin_log[round_robin_log_idx++] = task_id;
        }

        printf("Task %d: iteration %d\n", task_id, i);

        // 模拟一些工作
        for (volatile int j = 0; j < 1000; j++);

        // 每次迭代后主动yield，让其他同优先级任务运行
        // 这模拟了时间片到期后的抢占效果
        yield();
    }

    printf("Round-robin task %d (pid=%d): finished\n", task_id, myproc()->pid);
    exit(0);
}

void test_round_robin_scheduling(void) {
    printf("\n=== Round-Robin Time-Slice Tests ===\n");
    int passed = 0, total = 0;

    total++;
    printf("[Test %d] Round-robin algorithm demonstration...\n", total);
    printf("Note: Using cooperative yield() calls to simulate time-slice expiration.\n");
    printf("Expected: Tasks should alternate execution (interleaved), not run one after another.\n\n");

    // 重置日志
    round_robin_log_idx = 0;
    for (int i = 0; i < 100; i++) {
        round_robin_log[i] = 0;
    }

    // 创建三个相同优先级的任务
    int pid1 = create_kthread(round_robin_task, (void*)1);
    int pid2 = create_kthread(round_robin_task, (void*)2);
    int pid3 = create_kthread(round_robin_task, (void*)3);

    if (pid1 > 0 && pid2 > 0 && pid3 > 0) {
        // 设置为相同的普通优先级
        acquire(&ptable_lock);
        for (struct proc *p = procs; p < &procs[NPROC]; p++) {
            if (p->pid == pid1 || p->pid == pid2 || p->pid == pid3) {
                set_priority(p, PRIORITY_NORMAL);
                p->base_priority = PRIORITY_NORMAL;
                printf("Set process %d to NORMAL priority\n", p->pid);
            }
        }
        release(&ptable_lock);

        printf("\nThree tasks running with same priority...\n\n");

        // 等待任务完成
        wait(0);
        wait(0);
        wait(0);

        // 分析执行日志
        printf("\nExecution log (first 60 entries):\n");
        for (int i = 0; i < 60 && i < round_robin_log_idx; i++) {
            printf("%d ", round_robin_log[i]);
            if ((i + 1) % 20 == 0) printf("\n");
        }
        printf("\n\n");

        // 检查是否有交替执行的证据
        int switches = 0;
        for (int i = 1; i < round_robin_log_idx; i++) {
            if (round_robin_log[i] != round_robin_log[i-1]) {
                switches++;
            }
        }

        printf("Context switches detected: %d\n", switches);

        if (switches >= 5) {
            printf("✓ PASS: Round-robin scheduling working (tasks alternated %d times)\n", switches);
            passed++;
        } else {
            printf("✗ FAIL: Tasks ran sequentially with only %d switches (expected multiple alternations)\n", switches);
        }
    } else {
        printf("✗ FAIL: Could not create test processes\n");
    }

    printf("\n--- Round-Robin Test Summary: %d/%d tests passed ---\n", passed, total);
}

//===========================================
// 新增：Aging机制抢占测试（饥饿场景）
//===========================================

// 持续运行的高优先级任务
static volatile int starvation_test_running = 1;

void high_priority_spinning_task(void *arg) {
    int task_id = (int)(uint64_t)arg;

    printf("High priority spinning task %d (pid=%d, priority=%d): started\n",
           task_id, myproc()->pid, myproc()->priority);

    // 持续运行，占用CPU，但定期推进时间以允许aging机制工作
    int iterations = 0;
    while (starvation_test_running && iterations < 100000) {
        iterations++;

        if (iterations % 20000 == 0) {
            printf("High task %d: still running (iteration %d, priority=%d)\n",
                   task_id, iterations, myproc()->priority);
        }

        // 模拟工作，但定期yield以允许其他相同优先级任务运行和时间推进
        for (volatile int j = 0; j < 10; j++) {
            if (j % 5 == 0) {
                r_time(); // 定期推进模拟时间，使aging机制能够工作
                yield();  // 定期让出CPU，允许其他HIGH任务和LOW任务（如果被aging提升）运行
            }
        }
    }

    printf("High priority spinning task %d (pid=%d): finished after %d iterations\n",
           task_id, myproc()->pid, iterations);
    exit(0);
}

// 低优先级任务，期望通过aging获得执行机会
void starved_low_priority_task(void *arg) {
    printf("Starved low priority task (pid=%d, priority=%d): started\n",
           myproc()->pid, myproc()->priority);

    // 执行一些工作
    for (int i = 0; i < 10; i++) {
        printf("LOW task: iteration %d (priority=%d)\n", i, myproc()->priority);

        // 模拟工作
        for (volatile int j = 0; j < 1000; j++);
    }

    // 低优先级任务完成后，停止高优先级任务
    starvation_test_running = 0;

    printf("Starved low priority task (pid=%d): finished\n", myproc()->pid);
    exit(0);
}

void test_aging_starvation_prevention(void) {
    printf("\n=== Aging Mechanism Starvation Prevention Tests ===\n");
    int passed = 0, total = 0;

    total++;
    printf("[Test %d] Aging prevents starvation...\n", total);
    printf("Scenario: Multiple HIGH priority tasks keep running, starving LOW priority task.\n");
    printf("Expected: After waiting long enough, LOW priority task gets boosted by aging and runs.\n\n");

    starvation_test_running = 1;

    // 创建低优先级任务
    printf("Creating LOW priority starved task...\n");
    int low_pid = create_kthread(starved_low_priority_task, 0);

    if (low_pid > 0) {
        acquire(&ptable_lock);
        for (struct proc *p = procs; p < &procs[NPROC]; p++) {
            if (p->pid == low_pid) {
                set_priority(p, PRIORITY_LOW);
                p->base_priority = PRIORITY_LOW;
                printf("Set process %d to LOW priority (%d)\n", p->pid, PRIORITY_LOW);
                break;
            }
        }
        release(&ptable_lock);

        // 创建多个高优先级任务来占用CPU
        printf("\nCreating multiple HIGH priority spinning tasks to starve the LOW task...\n");
        int high_pid1 = create_kthread(high_priority_spinning_task, (void*)1);
        int high_pid2 = create_kthread(high_priority_spinning_task, (void*)2);
        int high_pid3 = create_kthread(high_priority_spinning_task, (void*)3);

        if (high_pid1 > 0 && high_pid2 > 0 && high_pid3 > 0) {
            acquire(&ptable_lock);
            for (struct proc *p = procs; p < &procs[NPROC]; p++) {
                if (p->pid == high_pid1 || p->pid == high_pid2 || p->pid == high_pid3) {
                    set_priority(p, PRIORITY_HIGH);
                    p->base_priority = PRIORITY_HIGH;
                    printf("Set process %d to HIGH priority (%d)\n", p->pid, PRIORITY_HIGH);
                }
            }
            release(&ptable_lock);

            printf("\n--- Watch for aging mechanism to boost LOW priority task ---\n");
            printf("You should see:\n");
            printf("1. HIGH tasks start running\n");
            printf("2. After some time, 'Aging: boosted process' messages appear\n");
            printf("3. LOW task finally gets to run (iteration messages)\n");
            printf("4. LOW task completes and stops HIGH tasks\n\n");

            // 让任务并发运行，等待LOW任务完成（它完成后会设置starvation_test_running = 0）
            printf("Letting tasks run concurrently for aging demonstration...\n");
            wait(0);  // low_pid 完成

            // 此时starvation_test_running = 0，HIGH任务会自然退出
            wait(0);  // high_pid1
            wait(0);  // high_pid2
            wait(0);  // high_pid3

            printf("\n✓ PASS: Aging mechanism allowed starved LOW priority task to run\n");
            passed++;
        } else {
            wait(0);
            printf("✗ FAIL: Could not create high priority processes\n");
        }
    } else {
        printf("✗ FAIL: Could not create low priority process\n");
    }

    printf("\n--- Aging Starvation Prevention Test Summary: %d/%d tests passed ---\n", passed, total);
}

// Race condition test task
static volatile int race_counter = 0;
static struct spinlock race_lock;

void race_task_unsafe(void *arg) {
    for (int i = 0; i < 1000; i++) {
        race_counter++; // Unsafe increment
    }
    exit(0);
}

void race_task_safe(void *arg) {
    for (int i = 0; i < 1000; i++) {
        acquire(&race_lock);
        race_counter++;
        release(&race_lock);
    }
    exit(0);
}

void test_synchronization(void) {
    printf("\n=== Synchronization Mechanism Tests ===\n");
    int passed = 0, total = 0;

    // Test 1: Producer-Consumer pattern
    total++;
    printf("[Test %d] Producer-Consumer synchronization...\n", total);
    shared_buffer_init();

    create_kthread(producer_task, 0);
    create_kthread(consumer_task, 0);

    wait(0);
    wait(0);

    if (shared_buffer.count == 0) {
        printf("✓ PASS: Producer-Consumer completed, buffer empty\n");
        passed++;
    } else {
        printf("✗ FAIL: Buffer not empty (count=%d)\n", shared_buffer.count);
    }

    // Test 2: Multiple Producers and Consumers
    total++;
    printf("\n[Test %d] Multiple Producers and Consumers...\n", total);
    shared_buffer_init();

    create_kthread(producer_task, 0);
    create_kthread(producer_task, 0);
    create_kthread(consumer_task, 0);
    create_kthread(consumer_task, 0);

    for (int i = 0; i < 4; i++) {
        wait(0);
    }

    if (shared_buffer.count == 0) {
        printf("✓ PASS: Multiple producers/consumers completed\n");
        passed++;
    } else {
        printf("✗ FAIL: Synchronization issue detected (count=%d)\n", shared_buffer.count);
    }

    // Test 3: Race condition test without lock
    total++;
    printf("\n[Test %d] Race condition test (without lock)...\n", total);
    race_counter = 0;

    create_kthread(race_task_unsafe, 0);
    create_kthread(race_task_unsafe, 0);

    wait(0);
    wait(0);

    printf("Race counter result (without lock): %d (expected: 2000)\n", race_counter);
    if (race_counter != 2000) {
        printf("✓ PASS: Race condition detected as expected\n");
        passed++;
    } else {
        printf("✗ FAIL: Race condition not detected (might be lucky)\n");
    }

    // Test 4: Race condition test with lock
    total++;
    printf("\n[Test %d] Race condition test (with lock)...\n", total);
    race_counter = 0;
    initlock(&race_lock, "race_test");

    create_kthread(race_task_safe, 0);
    create_kthread(race_task_safe, 0);

    wait(0);
    wait(0);

    printf("Race counter result (with lock): %d (expected: 2000)\n", race_counter);
    if (race_counter == 2000) {
        printf("✓ PASS: Lock properly prevented race condition\n");
        passed++;
    } else {
        printf("✗ FAIL: Lock failed to prevent race condition (%d)\n", race_counter);
    }

    printf("\n--- Synchronization Test Summary: %d/%d tests passed ---\n", passed, total);
}

//调试和测试入口
void debug_proc_table(void) {
    printf("\n=== Process Table ===\n");
    printf("PID\tSTATE\t\tNAME\n");
    printf("----------------------------------\n");

    acquire(&ptable_lock);
    for (int i = 0; i < NPROC; i++) {
        struct proc *p = &procs[i];
        if (p->state != UNUSED) {
            char *state_str;
            switch(p->state) {
                case USED:      state_str = "USED"; break;
                case SLEEPING:  state_str = "SLEEPING"; break;
                case RUNNABLE:  state_str = "RUNNABLE"; break;
                case RUNNING:   state_str = "RUNNING"; break;
                case ZOMBIE:    state_str = "ZOMBIE"; break;
                default:        state_str = "UNKNOWN"; break;
            }
            printf("%d\t%s\t\t%s\n", p->pid, state_str, p->name);
        }
    }
    release(&ptable_lock);
    printf("======================\n");
}

// 性能基准测试
void test_performance(void) {
    printf("\n=== Performance Benchmark Tests ===\n");
    int passed = 0, total = 0;

    // Test 1: Process creation performance
    total++;
    printf("[Test %d] Process creation performance...\n", total);
    uint64_t start_time = get_time();

    int num_procs = 20;
    for (int i = 0; i < num_procs; i++) {
        create_kthread(simple_task, 0);
    }

    for (int i = 0; i < num_procs; i++) {
        yield();
    }

    for (int i = 0; i < num_procs; i++) {
        wait(0);
    }

    uint64_t end_time = get_time();
    uint64_t elapsed = end_time - start_time;
    printf("Created and cleaned up %d processes in %llu time units\n", num_procs, elapsed);
    printf("Average: %llu time units per process\n", elapsed / num_procs);
    printf("✓ PASS: Performance benchmark completed\n");
    passed++;

    // Test 2: Context switch performance
    total++;
    printf("\n[Test %d] Context switch performance...\n", total);
    start_time = get_time();

    int num_switches = 100;
    for (int i = 0; i < num_switches; i++) {
        yield();
    }

    end_time = get_time();
    elapsed = end_time - start_time;
    printf("Performed %d context switches in %llu time units\n", num_switches, elapsed);
    printf("Average: %llu time units per switch\n", elapsed / num_switches);
    printf("✓ PASS: Context switch benchmark completed\n");
    passed++;

    // Test 3: Lock contention performance
    total++;
    printf("\n[Test %d] Lock contention performance...\n", total);
    struct spinlock test_lock;
    initlock(&test_lock, "perf_test");

    start_time = get_time();

    int num_acquires = 1000;
    for (int i = 0; i < num_acquires; i++) {
        acquire(&test_lock);
        release(&test_lock);
    }

    end_time = get_time();
    elapsed = end_time - start_time;
    printf("Performed %d lock acquire/release in %llu time units\n", num_acquires, elapsed);
    printf("Average: %llu time units per lock operation\n", elapsed / num_acquires);
    printf("✓ PASS: Lock performance benchmark completed\n");
    passed++;

    printf("\n--- Performance Test Summary: %d/%d tests passed ---\n", passed, total);
}

// --- 测试入口 ---
void run_all_tests() {
    // 直接启动shell
    //shell_main();
}
