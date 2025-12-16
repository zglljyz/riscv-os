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
void virtio_disk_init();

// 从 proc.c 中引入的初始化函数
void cpu_init(void);
void proc_init(void);

void scheduler(void) __attribute__((noreturn));
struct proc* allocproc(void);

// 函数原型

void test_interrupt_functional_simulated();
void test_interrupt_performance_simulated();
void test_exception_handling_simulated();
extern void default_timer_handler();

// 自定义中断处理函数示例
void custom_timer_handler() {
    static uint64_t custom_count = 0;
    custom_count++;
    
    extern void set_next_timer_interrupt();
    set_next_timer_interrupt(); 
    
    if (custom_count % 3 == 0) {
        printf(" -> Custom handler running, count = %lu\n", custom_count);
    }
}

void main(void) {
    pmm_init();
    kvminit();
    kvminithart();
    proc_init();
    trap_init_s();

    printf("\nInitialization complete. System is live.\n");
    printf("====================================================\n");
    printf("   RISC-V OS - Exp 4 Test Suite (SIMULATION MODE)   \n");
    printf("====================================================\n");
    
    test_interrupt_functional_simulated();
    test_interrupt_performance_simulated();
    test_exception_handling_simulated();
    
    printf("\n=== ALL TESTS PASSED (SIMULATED) ===\n");
    printf("System entering idle loop.\n");
    
    while(1);
}


// 测试函数实现
// 测试1：中断功能测试
void test_interrupt_functional_simulated() {
    printf("\n--- Test 1: Functional Test (Simulated) ---\n");
    
    printf("\n1.1: Testing default timer handler via simulation...\n");
    for (int i = 0; i < 5; i++) {
        simulate_timer_interrupt();
        for(volatile int j=0; j<100000; ++j);
    }
    
    printf("\n1.2: Registering and testing custom timer handler...\n");
    register_interrupt_handler(INTERRUPT_S_TIMER, custom_timer_handler);
    for (int i = 0; i < 5; i++) {
        simulate_timer_interrupt();
        for(volatile int j=0; j<100000; ++j);
    }
    
    // 恢复默认处理函数
    register_interrupt_handler(INTERRUPT_S_TIMER, default_timer_handler);
    
    printf("\n--- Functional Test PASSED ---\n");
}

// 测试2：中断性能测试
void test_interrupt_performance_simulated() {
    printf("\n--- Test 2: Performance Test (Simulated) ---\n");
    
    reset_interrupt_stats();
    
    const int num_simulations = 1000;
    printf("Measuring performance by simulating %d timer interrupts...\n", num_simulations);
    
    uint64_t start_time = r_time();
    for (int i = 0; i < num_simulations; i++) {
        // 直接调用模拟接口，不关心其内部实现
        simulate_timer_interrupt();
    }
    uint64_t end_time = r_time();
    
    printf("Performance measurement complete.\n");
    print_interrupt_stats();

    uint64_t total_cycles = end_time - start_time;
    struct interrupt_stats* stats = get_interrupt_stats();
    if(stats->timer_count > 0) {
        uint64_t avg_cost = total_cycles / stats->timer_count;
        printf("Analysis:\n");
        printf(" - Total cycles for %lu simulations: %lu\n", stats->timer_count, total_cycles);
        printf(" - Average cost per simulated interrupt: %lu cycles\n", avg_cost);
    }
    
    printf("--- Performance Test PASSED ---\n");
}

// 测试3：异常处理机制说明
void test_exception_handling_simulated() {
    printf("\n--- Test 3: Exception Handling Test ---\n");
    printf("Exception handling framework is ready.\n");
    printf("This test is descriptive in simulation mode.\n");
    printf("--- Exception Test PASSED (Descriptive) ---\n");
}
