#pragma once
#include "types.h"

/****************************************************************/
/*                      分页相关的宏定义                          */
/****************************************************************/

#define PGSIZE 4096       // 页大小，4KB
#define PGSHIFT 12        // 页大小对应的幂次 (2^12 = 4096)

// 将地址对齐
#define PGROUNDDOWN(a) (((a)) & ~(PGSIZE - 1))
#define PGROUNDUP(a) (PGROUNDDOWN((a) + PGSIZE - 1))

// 从虚拟地址中提取各级页表的索引 (VPN)
#define VPN_INDEX(va, level) ( ((uint64_t)(va) >> (PGSHIFT + 9 * (level))) & 0x1FF )

// 根据根页表地址构造 satp 寄存器的值 (Sv39 模式)
#define MAKE_SATP(pt) ( (8L << 60) | (((uint64_t)pt) >> 12) )


/****************************************************************/
/*                      内存布局相关的宏                         */
/****************************************************************/

// Sv39 分页模式下的最大有效虚拟地址
#define MAXVA (1L << (9 + 9 + 9 + 12 - 1))

// 内核基地址，必须与链接脚本 (kernel.ld) 中的地址保持一致
#define KERNBASE 0x80000000L

// QEMU virt 机器中 UART 设备的物理地址
#define UARTO 0x10000000L

// Trampoline page - mapped at same VA in both kernel and user page tables
// Must be at the top of virtual address space (one page below MAXVA)
#define TRAMPOLINE (MAXVA - PGSIZE)

// Trapframe page - mapped at same VA in both kernel and user page tables
// Just below TRAMPOLINE
#define TRAPFRAME (TRAMPOLINE - PGSIZE)


/****************************************************************/
/*               CSR 寄存器读写函数 (内联汇编)                   */
/****************************************************************/

// --- 通用 CSR 读写宏 ---
#define csr_read(csr)                                           \
    ({                                                          \
        uint64_t __v;                                           \
        asm volatile("csrr %0, " #csr : "=r"(__v) : : "memory"); \
        __v;                                                    \
    })

#define csr_write(csr, val)                                        \
    ({                                                             \
        uint64_t __v = (uint64_t)(val);                           \
        asm volatile("csrw " #csr ", %0" : : "r"(__v) : "memory"); \
    })


// --- 特定 CSR 的便捷函数 ---
static inline void w_stvec(uint64_t x) { csr_write(stvec, x); }
static inline uint64_t r_sstatus() { return csr_read(sstatus); }
static inline void w_sstatus(uint64_t x) { csr_write(sstatus, x); }
static inline uint64_t r_scause() { return csr_read(scause); }
static inline uint64_t r_sepc() { return csr_read(sepc); }
static inline void w_sepc(uint64_t x) { csr_write(sepc, x); }
static inline void w_sie(uint64_t x) { csr_write(sie, x); }
static inline uint64_t r_sie() { return csr_read(sie); }
static inline void w_satp(uint64_t x) { csr_write(satp, x); }
static inline uint64_t r_satp() { return csr_read(satp); }
static inline void w_sscratch(uint64_t x) {
    asm volatile("csrw sscratch, %0" : : "r" (x));
}
// 读取硬件时钟周期计数器，提供真正的时间源
static inline uint64_t r_time() {
    uint64_t cycles;
    asm volatile("rdcycle %0" : "=r"(cycles));
    return cycles;
}
static inline uint64_t r_stval() { return csr_read(stval); }

/****************************************************************/
/*                   中断相关的宏定义                           */
/****************************************************************/

// Supervisor Status Register, sstatus
#define SSTATUS_SIE (1L << 1) // Supervisor Interrupt Enable
#define SSTATUS_SPP (1L << 8)  // Supervisor Previous Privilege (0=User, 1=Supervisor)
#define SSTATUS_SPIE (1L << 5)  // Supervisor Previous Interrupt Enable
#define SSTATUS_SUM (1L << 18)  // Permit Supervisor User Memory access
// Supervisor Interrupt Enable Register, sie
#define SIE_SEIE (1L << 9) // Supervisor External Interrupt Enable
#define SIE_STIE (1L << 5) // Supervisor Timer Interrupt Enable
#define SIE_SSIE (1L << 1) // Supervisor Software Interrupt Enable

// --- 中断号定义 ---
// scause register interrupt codes
#define INTERRUPT_S_SOFTWARE    1
#define INTERRUPT_S_TIMER       5
#define INTERRUPT_S_EXTERNAL    9
