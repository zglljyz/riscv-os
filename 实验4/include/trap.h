#pragma once
#include "types.h"

// trapframe 结构体，用于在中断/异常时保存所有通用寄存器
// 这个结构体在 kernelvec.s 中被填充，并作为参数传递给 C 语言的 trap 处理函数
struct trapframe {
    // a0-a7, s0-s11, t0-t6, ra, sp, gp, tp
    // 按顺序保存所有32个通用寄存器，除了 x0(zero)
    uint64_t ra;
    uint64_t sp;
    uint64_t gp;
    uint64_t tp;
    uint64_t t0, t1, t2;
    uint64_t s0, s1;
    uint64_t a0, a1, a2, a3, a4, a5, a6, a7;
    uint64_t s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
    uint64_t t3, t4, t5, t6;
    uint64_t epc;
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
