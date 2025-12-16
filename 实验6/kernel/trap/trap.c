#include "riscv.h"
#include "trap.h"
#include "sbi.h"
#include "proc.h"  
#include "cpu.h"   
#include "syscall.h"
void printf(const char *fmt, ...);

// 声明在 kernelvec.s 中定义的汇编入口函数
extern void kernelvec();
// 声明在 trampoline.s 中定义的函数和符号
extern char trampoline[];  // trampoline page start
extern char uservec[];     // user trap entry point
extern char userret[];     // return to user mode
static void intr_off() {
    w_sstatus(r_sstatus() & ~SSTATUS_SIE);
}
// 用于记录时钟中断次数的计数器
static uint64_t ticks = 0;

// 定义时钟中断的间隔（周期数）
#define TIMER_INTERVAL 1000000000 // 增加时间间隔到1B周期，避免过于频繁的中断

// 中断处理函数表
static interrupt_handler_t interrupt_handlers[16] = {0};

// 异常处理函数表  
static exception_handler_t exception_handlers[16] = {0};

// 中断统计数据
static struct interrupt_stats stats = {0};

void usertrap(void) {
    struct proc *p = myproc();

    // 确保是从用户模式来的
    if ((r_sstatus() & SSTATUS_SPP) != 0) {
        printf("usertrap: not from user mode\n");
        while(1);
    }

    // 发送中断和异常到此处（用户模式）
    w_stvec((uint64_t)kernelvec);

    // 保存用户程序计数器
    p->trapframe->epc = r_sepc();

    uint64_t scause = r_scause();

    if (scause == 8) {
        // 系统调用（ecall from U-mode）
        // sepc 指向 ecall 指令，需要+4 跳过它
        p->trapframe->epc += 4;

        // 处理系统调用
        syscall();
    }
    else if (scause & (1UL << 63)) {
        // 中断
        uint64_t irq = scause & 0xff;

        if (irq == 5) {
            // 时钟中断
            stats.timer_count++;
            set_next_timer_interrupt();
            yield();
        }
        else {
            printf("usertrap: unexpected interrupt scause 0x%lx\n", scause);
            printf("  sepc=0x%lx stval=0x%lx\n", r_sepc(), r_stval());
        }
    }
    else {
        // 异常
        printf("usertrap: unexpected exception scause 0x%lx\n", scause);
        printf("  sepc=0x%lx stval=0x%lx\n", r_sepc(), r_stval());
        printf("  pid=%d\n", p->pid);
        p->killed = 1;
    }

    if (p->killed) {
        exit(-1);
    }

    usertrapret();
}

// 设置下一次时钟中断
void set_next_timer_interrupt() {
    // printf("[set_next_timer_interrupt] Entering function\n");
    uint64_t current_time = r_time();
    // printf("[set_next_timer_interrupt] Current time: %ld\n", current_time);
    uint64_t target_time = current_time + TIMER_INTERVAL;
    // printf("[set_next_timer_interrupt] Target time: %ld\n", target_time);

    // 通过SBI调用设置mtimecmp
    // printf("[set_next_timer_interrupt] About to call sbi_set_timer\n");
    sbi_set_timer(target_time);
    // printf("[set_next_timer_interrupt] sbi_set_timer returned\n");
}

// 默认时钟中断处理函数
static void default_timer_handler() {
    ticks++;
    stats.timer_count++;
    
    // 设置下一次时钟中断
    set_next_timer_interrupt();
    
    if (ticks % 100 == 0) { // 每100次中断打印一次，避免刷屏
        printf("Timer interrupt! ticks = %lu\n", ticks);
    }
}

// 默认异常处理函数
static void default_exception_handler(uint64_t cause, uint64_t epc, uint64_t tval) {
    printf("=== UNHANDLED EXCEPTION ===\n");
    printf("Exception code: %lu\n", cause);
    printf("Exception PC (sepc): 0x%lx\n", epc);
    printf("Exception value (stval): 0x%lx\n", tval);
    
    // 根据异常类型提供更详细的信息
    switch(cause) {
        case 0:
            printf("Instruction address misaligned\n");
            break;
        case 1:
            printf("Instruction access fault\n");
            break;
        case 2:
            printf("Illegal instruction\n");
            break;
        case 3:
            printf("Breakpoint\n");
            break;
        case 4:
            printf("Load address misaligned\n");
            break;
        case 5:
            printf("Load access fault\n");
            break;
        case 6:
            printf("Store/AMO address misaligned\n");
            break;
        case 7:
            printf("Store/AMO access fault\n");
            break;
        case 8:
            printf("Environment call from U-mode\n");
            break;
        case 9:
            printf("Environment call from S-mode\n");
            break;
        case 12:
            printf("Instruction page fault\n");
            break;
        case 13:
            printf("Load page fault\n");
            break;
        case 15:
            printf("Store/AMO page fault\n");
            break;
        default:
            printf("Unknown exception\n");
            break;
    }
    
    printf("System halted.\n");
    while (1); // 停机
}

// 注册中断处理函数
void register_interrupt_handler(int irq, interrupt_handler_t handler) {
    if (irq >= 0 && irq < 16) {
        interrupt_handlers[irq] = handler;
        printf("Registered interrupt handler for IRQ %d\n", irq);
    } else {
        printf("Invalid IRQ number: %d\n", irq);
    }
}

// 注册异常处理函数
void register_exception_handler(int exception_code, exception_handler_t handler) {
    if (exception_code >= 0 && exception_code < 16) {
        exception_handlers[exception_code] = handler;
        printf("Registered exception handler for code %d\n", exception_code);
    } else {
        printf("Invalid exception code: %d\n", exception_code);
    }
}

// 打印中断统计信息
void print_interrupt_stats() {
    printf("\n=== INTERRUPT STATISTICS ===\n");
    printf("Timer interrupts: %lu\n", stats.timer_count);
    printf("External interrupts: %lu\n", stats.external_count);
    printf("Software interrupts: %lu\n", stats.software_count);
    printf("Exceptions: %lu\n", stats.exception_count);
    printf("Total interrupts: %lu\n", stats.total_interrupts);
    printf("Max handler time: %lu cycles\n", stats.max_handler_time);
    printf("Min handler time: %lu cycles\n", stats.min_handler_time);
    printf("=============================\n\n");
}

// 重置中断统计信息
void reset_interrupt_stats() {
    stats.timer_count = 0;
    stats.external_count = 0;
    stats.software_count = 0;
    stats.exception_count = 0;
    stats.total_interrupts = 0;
    stats.max_handler_time = 0;
    stats.min_handler_time = UINT64_MAX;
    printf("Interrupt statistics reset.\n");
}

// S-mode trap 初始化函数
void trap_init_s() {
    printf("Initializing S-mode trap handler...\n");

    // 设置中断向量表地址到stvec寄存器
    w_stvec((uint64_t)kernelvec);
    printf("stvec set to 0x%lx\n", (uint64_t)kernelvec);

    // 开启S-mode时钟中断
    w_sie(r_sie() | SIE_STIE);
    printf("SIE enabled with STIE\n");

    // 开启全局中断
    w_sstatus(r_sstatus() | SSTATUS_SIE);
    printf("Global S-mode interrupts enabled\n");

    // 注册时钟中断处理函数
    register_interrupt_handler(INTERRUPT_S_TIMER, default_timer_handler);

    // 初始化统计数据
    reset_interrupt_stats();

    // 不立即设置时钟中断，等到进程创建后再设置
    printf("S-mode trap handler initialized (timer will be started later).\n");
    printf("Interrupt framework ready.\n");
}

// 启动时钟中断（由调度器调用）
void start_timer_interrupts() {
    printf("Starting timer interrupts...\n");
    set_next_timer_interrupt();
    printf("Timer interrupts started.\n");
}



void kerneltrap() {
    uint64_t scause = r_scause();
    uint64_t sepc = r_sepc();
    uint64_t sstatus = r_sstatus();
    struct proc *p = myproc(); // 获取当前进程指针

    // 检查最高位，判断是中断还是异常
    if (scause & (1UL << 63)) {
        // --- 这是内核态中断 ---
        uint64_t interrupt_type = scause & 0x7FFFFFFFFFFFFFFF;

        switch (interrupt_type) {
            case INTERRUPT_S_TIMER: // Supervisor Timer Interrupt
                ticks++;
                stats.timer_count++;

                // 时钟中断驱动抢占式调度
                if (p != 0 && p->state == RUNNING) {
                    yield();
                }
                set_next_timer_interrupt();

                // 每100次中断打印一次，避免刷屏
                if (ticks % 100 == 0) {
                    printf("Timer interrupt! ticks = %lu (hardware)\n", ticks);
                }
                break;

            case INTERRUPT_S_EXTERNAL: // Supervisor External Interrupt
                // 处理外部设备中断...
                break;

            default:
                // 处理未知的内核态中断
                printf("kerneltrap: unexpected kernel interrupt scause %p, sepc %p\n", scause, sepc);
                while(1);
        }
    } else {
        // --- 这是内核态异常 ---
        // 检查是否是来自内核的SBI调用 (这是正常的)
        if (scause == 9) {  // Environment call from S-mode
            // 这是SBI调用，静默处理，不需要打印
            // SBI调用会由硬件/固件处理，我们只需要继续执行
        } else {
            printf("kerneltrap: kernel exception scause %p, sepc %p\n", scause, sepc);
            // 检查中断状态
            if ((sstatus & SSTATUS_SIE) == 0) {
                printf("kerneltrap: interrupts were disabled\n");
            }
            printf("sstatus: %p\n", sstatus);
            // 内核代码不应该产生异常，直接停机。
            while(1);
        }
    }
}

// 获取中断统计信息指针
struct interrupt_stats* get_interrupt_stats() {
    return &stats;
}

void usertrapret() {
    struct proc *p = myproc();

    // 1. 关中断，直到 sret 原子地恢复 sstatus
    intr_off();

    // 1.5 xv6关键步骤:将当前进程的trapframe映射到内核页表的TRAPFRAME地址
    // 这样userret可以通过TRAPFRAME访问trapframe
    extern pagetable_t kernel_pagetable;
    extern pte_t *walk(pagetable_t, uint64_t, int);
    extern int map_page(pagetable_t, uint64_t, uint64_t, int);

    // 检查TRAPFRAME是否已经映射
    pte_t *pte = walk(kernel_pagetable, TRAPFRAME, 0);
    if (pte == 0 || (*pte & PTE_V) == 0) {
        // 未映射,需要映射
        if (map_page(kernel_pagetable, TRAPFRAME, (uint64_t)p->trapframe, PTE_R | PTE_W) != 0) {
            printf("FATAL: cannot map TRAPFRAME in kernel page table\n");
            while(1);
        }
        // 刷新TLB - 这是xv6的关键步骤!
        asm volatile("sfence.vma zero, zero");
    } else {
        // 已映射,检查是否映射到正确的物理地址
        uint64_t mapped_pa = PTE2PA(*pte);
        if (mapped_pa != (uint64_t)p->trapframe) {
            // 映射到了不同的物理地址,需要更新
            *pte = PA2PTE((uint64_t)p->trapframe) | PTE_R | PTE_W | PTE_V;
            // 刷新TLB
            asm volatile("sfence.vma zero, zero");
        }
    }

    // 2. 设置stvec指向trampoline中的uservec（处理用户态trap）
    extern char trampoline[], uservec[];
    uint64_t uservec_offset = (uint64_t)uservec - (uint64_t)trampoline;
    w_stvec(TRAMPOLINE + uservec_offset);

    // 3. 填充trapframe的kernel字段 (xv6布局)，供trampoline使用
    extern pagetable_t kernel_pagetable;
    p->trapframe->kernel_satp = MAKE_SATP(kernel_pagetable);  // 内核页表 (offset 0)
    p->trapframe->kernel_sp = p->kstack + 4096;  // 内核栈顶 (offset 8)
    extern void usertrap();
    p->trapframe->kernel_trap = (uint64_t)usertrap;  // usertrap函数地址 (offset 16)
    p->trapframe->user_satp = MAKE_SATP(p->pagetable);  // 用户页表 (offset 288)

    // 4. 设置 sstatus 寄存器，准备返回 U-mode
    //    - 清除 SPP 位 (表示 sret 后将进入 U-mode)
    //    - 设置 SPIE 位 (表示 sret 后 U-mode 的中断是开启的)
    uint64_t x = r_sstatus();
    x &= ~SSTATUS_SPP; // Clear SPP
    x |= SSTATUS_SPIE; // Set SPIE
    w_sstatus(x);

    // 5. 设置用户程序的入口地址到 sepc
    w_sepc(p->trapframe->epc);

    // 6. 设置sscratch指向TRAPFRAME虚拟地址 (xv6方式)
    //    sscratch需要是在用户页表中有效的地址
    w_sscratch(TRAPFRAME);

    // 7. 调用trampoline中的userret来返回用户态
    // 关键：传递TRAPFRAME虚拟地址，而不是物理地址！
    // TRAPFRAME在两个页表中都有效：
    //   - 在内核页表中，TRAPFRAME映射到trapframe物理页
    //   - 在用户页表中，TRAPFRAME也映射到同一个trapframe物理页
    // 这样userret可以安全地切换页表
    extern char trampoline[], userret[];
    uint64_t userret_offset = (uint64_t)userret - (uint64_t)trampoline;
    uint64_t fn = TRAMPOLINE + userret_offset;

    // xv6关键点: 传递user_satp值(不是TRAPFRAME地址!)
    uint64_t user_satp = p->trapframe->user_satp;

    // xv6方式: 调用trampoline中的userret,传递satp值
    ((void(*)(uint64_t))fn)(user_satp);
}


// 模拟中断触发函数，用于测试中断处理框架
void simulate_timer_interrupt(void) {
    printf("模拟时钟中断触发\n");
    stats.timer_count++;
    stats.total_interrupts++;
    
    if (interrupt_handlers[INTERRUPT_S_TIMER]) {
        interrupt_handlers[INTERRUPT_S_TIMER]();
    } else {
        default_timer_handler();
    }
}

void simulate_external_interrupt(void) {
    printf("模拟外部中断触发\n");
    stats.external_count++;
    stats.total_interrupts++;
    
    if (interrupt_handlers[INTERRUPT_S_EXTERNAL]) {
        interrupt_handlers[INTERRUPT_S_EXTERNAL]();
    } else {
        printf("处理外部中断\n");
    }
}

void simulate_software_interrupt(void) {
    printf("模拟软件中断触发\n");
    stats.software_count++;
    stats.total_interrupts++;
    
    if (interrupt_handlers[INTERRUPT_S_SOFTWARE]) {
        interrupt_handlers[INTERRUPT_S_SOFTWARE]();
    } else {
        printf("处理软件中断\n");
    }
}
