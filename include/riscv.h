
#pragma once // 防止头文件被重复包含

// 页大小为4KB
#define PGSIZE 4096
// 页大小的幂次 (2^12 = 4096)
#define PGSHIFT 12

// 将地址向上舍入到页边界
#define PGROUNDUP(addr) (((addr) + PGSIZE - 1) & ~(PGSIZE - 1))
// 将地址向下舍入到页边界
#define PGROUNDDOWN(addr) ((addr) & ~(PGSIZE - 1))
