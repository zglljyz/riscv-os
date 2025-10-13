
#include "riscv.h"
#include "pmm.h"
#include "vm.h"
#include "string.h"
void printf(const char *fmt, ...);


// 遍历页表以找到虚拟地址va对应的PTE地址。
// 如果alloc为1，则在需要时分配新的页表页。
pte_t *walk(pagetable_t pagetable, uint64_t va, int alloc) {
    // 虚拟地址必须低于MAXVA (Sv39的最大虚拟地址)
    if (va >= (1L << 39)) {
        return 0;
    }

    for (int level = 2; level > 0; level--) {
        pte_t *pte = &pagetable[VPN_INDEX(va, level)];

        if (*pte & PTE_V) {
            // PTE有效，直接进入下一级
            pagetable = (pagetable_t)PTE2PA(*pte);
        } else {
            // PTE无效，需要分配新的页表
            if (!alloc) {
                return 0; // 不允许分配，则返回失败
            }
            // 分配一个新页作为下一级页表
            pagetable = (pagetable_t)alloc_page();
            if (pagetable == 0) {
                return 0; // 内存不足
            }
            // 将新页清零，所有PTE都将是无效的
            memset(pagetable, 0, PGSIZE);
            // 更新当前PTE，使其指向新分配的页表
            *pte = PA2PTE(pagetable) | PTE_V;
        }
    }
    // 返回最终的L0 PTE的地址
    return &pagetable[VPN_INDEX(va, 0)];
}


// 创建一个虚拟地址va到物理地址pa的映射
int map_page(pagetable_t pagetable, uint64_t va, uint64_t pa, int perm) {
    // 确保地址是页对齐的
    if (PGROUNDDOWN(va) != va || PGROUNDDOWN(pa) != pa) {
        return -1;
    }

    pte_t *pte = walk(pagetable, va, 1); // 查找PTE，如果需要则分配页表
    if (pte == 0) {
        return -1; // walk失败
    }
    if (*pte & PTE_V) {
        // 页面已经被映射过了，这是一个错误

        return -1;
    }

    // 设置PTE内容：物理地址 + 权限位 + 有效位
    *pte = PA2PTE(pa) | perm | PTE_V;
    return 0;
}


// 创建一个空的页表
pagetable_t create_pagetable() {
    pagetable_t pagetable = (pagetable_t)alloc_page();
    if (pagetable) {
        memset(pagetable, 0, PGSIZE);
    }
    return pagetable;
}

// 外部符号声明
extern char etext[];    // 内核代码段结束的位置
extern char PHYSTOP[];  // 物理内存顶部
extern char end[];      // 内核数据段结束的位置

// UART设备的物理地址
#define UART0 0x10000000L

// 内核页表
static pagetable_t kernel_pagetable;

// ========= 大页支持 =========
// 映射一个2MB的大页
static int map_megapage(pagetable_t pagetable, uint64_t va, uint64_t pa, int perm) {
    // 大页映射发生在L2页表，只需要走一级
    pte_t *pte = &pagetable[VPN_INDEX(va, 2)];

    if (*pte & PTE_V) { // L2 PTE不应该提前存在
        return -1;
    }

    // 设置为叶子PTE：物理地址 + 权限 + 有效位
    *pte = PA2PTE(pa) | perm | PTE_V;
    return 0;
}


static void map_region(pagetable_t pt, uint64_t va_start, uint64_t va_end, uint64_t pa_start, int perm) {
    uint64_t va = va_start;
    uint64_t pa = pa_start;

    while (va < va_end) {
        // 如果当前VA, PA都已2MB对齐，并且剩余空间大于等于2MB
        if (IS_MEGAPAGE_ALIGNED(va) && IS_MEGAPAGE_ALIGNED(pa) && (va_end - va) >= MEGAPAGE_SIZE) {
            // 使用大页进行映射
            map_megapage(pt, va, pa, perm);
            va += MEGAPAGE_SIZE;
            pa += MEGAPAGE_SIZE;
        } else {
            // 否则，回退到使用4KB小页
            map_page(pt, va, pa, perm);
            va += PGSIZE;
            pa += PGSIZE;
        }
    }
}


void kvminit() {
    kernel_pagetable = create_pagetable();
    
    // 需要映射的区域：
    // 1. UART设备: [0x10000000, 0x10000000 + PGSIZE] -> R/W
    // 2. 内核代码+数据: [end, PHYSTOP]
    // 其中内核代码(.text)部分在[end, etext] -> R/X
    // 数据部分(.data, .bss, etc)在[etext, PHYSTOP] -> R/W
    
    printf("Mapping UART...\n");
    map_page(kernel_pagetable, UART0, UART0, PTE_R | PTE_W);
    
    printf("Mapping Kernel Code (.text)...\n");
    // 假设 end 地址和 etext 地址可能不对齐，所以用 map_region
    // map_region内部会自动选择最优的页大小
    map_region(kernel_pagetable, (uint64_t)end, (uint64_t)etext, (uint64_t)end, PTE_R | PTE_X);
    
    printf("Mapping Kernel Data and remaining RAM...\n");
    map_region(kernel_pagetable, (uint64_t)etext, (uint64_t)PHYSTOP, (uint64_t)etext, PTE_R | PTE_W);
}

// 写入satp寄存器，开启分页
void kvminithart() {
    // 构造SATP寄存器的值
    // 模式(MODE) = 8 (Sv39) | (根页表地址 >> 12)
    uint64_t satp = (8L << 60) | (((uint64_t)kernel_pagetable) >> 12);

    // 通过内联汇编写入SATP寄存器
    asm volatile("csrw satp, %0" : : "r"(satp));
    
    // 刷新TLB
    asm volatile("sfence.vma zero, zero");

    printf("Paging enabled!\n");
}


//Page Table 单元测试函数

// 一个简单的断言宏，用于测试
#define assert(expr)                                    \
    if (!(expr)) {                                      \
        printf("Assertion failed: %s, file %s, line %d\n", #expr, __FILE__, __LINE__); \
        while (1);                                      \
    }


void test_pagetable() {
    printf("\n--- Starting Page Table Unit Tests ---\n");

    // 1. 创建页表
    pagetable_t pt = create_pagetable();
    assert(pt != 0);
    printf("   Created a new page table at %p\n", pt);

    // 2. 测试基本映射
    uint64_t va = 0x100000; // 一个任意的虚拟地址
    void* pa_ptr = alloc_page();
    assert(pa_ptr != 0);
    uint64_t pa = (uint64_t)pa_ptr;
    
    printf("   Mapping va=0x%lx to pa=%p\n", va, (void*)pa); 
    int ret = map_page(pt, va, pa, PTE_R | PTE_W);
    assert(ret == 0);

    // 3. 测试 walk 函数来查找映射
    pte_t *pte = walk(pt, va, 0); // 用 alloc=0 来查找
    assert(pte != 0);
    assert(*pte & PTE_V);
    assert((*pte & (PTE_R | PTE_W)) == (PTE_R | PTE_W));
    assert(!(*pte & PTE_X));
    assert(PTE2PA(*pte) == pa);
    
    printf("   Success: walk() found the correct PTE with correct PA and permissions.\n");
    
    // 清理
    free_page(pa_ptr); 


    printf("--- Page Table Unit Tests Finished ---\n");
}
