#pragma once
#include "types.h"

// --- VM 相关类型定义 ---
typedef uint64_t *pagetable_t;
typedef uint64_t pte_t;

// --- PTE (页表项) 标志位 ---
#define PTE_V (1L << 0) // 有效位 (Valid)
#define PTE_R (1L << 1) // 可读位 (Read)
#define PTE_W (1L << 2) // 可写位 (Write)
#define PTE_X (1L << 3) // 可执行位 (Execute)
#define PTE_U (1L << 4) // 用户位 (User)
#define PTE_FLAGS(pte) ((pte) & 0x3FF) // PTE的低10位是标志位
// --- 地址翻译相关的宏 ---
// riscv.h 应该已经定义了 PGSHIFT = 12

// 将物理地址转换为 PTE 中的物理页号 (PPN)
// pa >> 12 << 10
#define PA2PTE(pa) ((((uint64_t)pa) >> 12) << 10)

// 将 PTE 中的物理页号 (PPN) 转换为物理地址
// pte >> 10 << 12
#define PTE2PA(pte) ((((pte) >> 10) << 12))

// --- 大页 (Megapage) 相关的宏 ---
#define MEGAPAGE_SIZE (2 * 1024 * 1024) // 2MB

// 检查地址是否是 2MB 对齐的
#define IS_MEGAPAGE_ALIGNED(addr) (((uint64_t)(addr) % MEGAPAGE_SIZE) == 0)


// --- 函数原型 ---
void kvminit();
void kvminithart();
pte_t *walk(pagetable_t pagetable, uint64_t va, int alloc);
int map_page(pagetable_t pagetable, uint64_t va, uint64_t pa, int perm);

// 测试函数原型
void test_pagetable();
void test_page_table_remap();
void test_kernel_mapping();
