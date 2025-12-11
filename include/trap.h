#pragma once
#include "types.h"

// trapframe 结构体 - xv6布局
// 这个结构体在 trampoline.s 中被填充，并作为参数传递给 C 语言的 trap 处理函数
// IMPORTANT: 必须和xv6保持完全一致的布局！
struct trapframe {
    // Kernel fields FIRST (accessed by trampoline code)
    /*   0 */ uint64_t kernel_satp;   // kernel page table
    /*   8 */ uint64_t kernel_sp;     // top of process's kernel stack
    /*  16 */ uint64_t kernel_trap;   // usertrap()
    /*  24 */ uint64_t epc;           // saved user program counter
    /*  32 */ uint64_t kernel_hartid; // saved kernel tp (not used yet)

    // User registers (saved/restored by trampoline)
    /*  40 */ uint64_t ra;
    /*  48 */ uint64_t sp;
    /*  56 */ uint64_t gp;
    /*  64 */ uint64_t tp;
    /*  72 */ uint64_t t0;
    /*  80 */ uint64_t t1;
    /*  88 */ uint64_t t2;
    /*  96 */ uint64_t s0;
    /* 104 */ uint64_t s1;
    /* 112 */ uint64_t a0;
    /* 120 */ uint64_t a1;
    /* 128 */ uint64_t a2;
    /* 136 */ uint64_t a3;
    /* 144 */ uint64_t a4;
    /* 152 */ uint64_t a5;
    /* 160 */ uint64_t a6;
    /* 168 */ uint64_t a7;
    /* 176 */ uint64_t s2;
    /* 184 */ uint64_t s3;
    /* 192 */ uint64_t s4;
    /* 200 */ uint64_t s5;
    /* 208 */ uint64_t s6;
    /* 216 */ uint64_t s7;
    /* 224 */ uint64_t s8;
    /* 232 */ uint64_t s9;
    /* 240 */ uint64_t s10;
    /* 248 */ uint64_t s11;
    /* 256 */ uint64_t t3;
    /* 264 */ uint64_t t4;
    /* 272 */ uint64_t t5;
    /* 280 */ uint64_t t6;

    // Additional field for user page table
    /* 288 */ uint64_t user_satp;     // user page table
};

// 中断处理函数类型定义
typedef void (*interrupt_handler_t)(void);

// 异常处理函数类型定义  
typedef void (*exception_handler_t)(uint64_t cause, uint64_t epc, uint64_t tval);

// 中断统计结构
struct interrupt_stats {
    uint64_t timer_count;      // 时钟中断次数
    uint64_t external_count;   // 外部中断次数  
    uint64_t software_count;   // 软件中断次数
    uint64_t exception_count;  // 异常次数
    uint64_t total_interrupts; // 总中断次数
    uint64_t max_handler_time; // 最大处理时间
    uint64_t min_handler_time; // 最小处理时间
};

// 函数原型
void trap_init_s();
void start_timer_interrupts();  // 启动时钟中断
void kerneltrap();
void register_interrupt_handler(int irq, interrupt_handler_t handler);
void register_exception_handler(int exception_code, exception_handler_t handler);
void print_interrupt_stats();
void reset_interrupt_stats();
struct interrupt_stats* get_interrupt_stats(); // 获取统计信息指针

// 添加模拟中断触发函数
void simulate_timer_interrupt(void);
void simulate_external_interrupt(void);
void simulate_software_interrupt(void);
