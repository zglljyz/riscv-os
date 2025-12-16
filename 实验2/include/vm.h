
#pragma once
#include "riscv.h" // 需要PTE相关的位定义
#include "types.h"
typedef uint64_t *pagetable_t; // 页表是一个指向64位PTE数组的指针
typedef uint64_t pte_t;       // 一个PTE是64位无符号整数

// RISC-V PTE标志位 (来自特权级手册)
#define PTE_V (1L << 0) // Valid
#define PTE_R (1L << 1) // Read
#define PTE_W (1L << 2) // Write
#define PTE_X (1L << 3) // Execute
#define PTE_U (1L << 4) // User

// 将物理地址转换为PTE中的PPN (物理页号)
#define PA2PTE(pa) ((((uint64_t)pa) >> 12) << 10)

// 将PTE中的PPN转换为物理地址
#define PTE2PA(pte) ((((pte) >> 10) << 12))

// 从虚拟地址中提取各级VPN索引
#define VPN_SHIFT(level) (PGSHIFT + 9 * (level))
#define VPN_INDEX(va, level) (((uint64_t)(va) >> VPN_SHIFT(level)) & 0x1FF)

// 2MB大页大小
#define MEGAPAGE_SIZE (2 * 1024 * 1024)

// 检查地址是否是2MB对齐的
#define IS_MEGAPAGE_ALIGNED(addr) (((uint64_t)(addr) % MEGAPAGE_SIZE) == 0)

// 函数原型
pagetable_t create_pagetable();
pte_t *walk(pagetable_t pagetable, uint64_t va, int alloc);
int map_page(pagetable_t pagetable, uint64_t va, uint64_t pa, int perm);


void kvminit();
void kvminithart();

void test_pagetable();
