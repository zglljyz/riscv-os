#include "riscv.h"
#include "pmm.h"
#include "vm.h"
#include "string.h"

void printf(const char *fmt, ...);

// 外部符号，由链接脚本提供
extern char etext[];
extern char PHYSTOP[];

// 全局内核页表 - 改为非static以便其他文件访问
pagetable_t kernel_pagetable;

// walk, map_page, create_pagetable 函数 (你的版本是正确的，保持不变)
pte_t *walk(pagetable_t pagetable, uint64_t va, int alloc) {
    if (va >= MAXVA) return 0;
    for (int level = 2; level > 0; level--) {
        pte_t *pte = &pagetable[VPN_INDEX(va, level)];
        if (*pte & PTE_V) {
            pagetable = (pagetable_t)PTE2PA(*pte);
        } else {
            if (!alloc || (pagetable = (pagetable_t)alloc_page()) == 0) return 0;
            memset(pagetable, 0, PGSIZE);
            *pte = PA2PTE(pagetable) | PTE_V;
        }
    }
    return &pagetable[VPN_INDEX(va, 0)];
}

int map_page(pagetable_t pagetable, uint64_t va, uint64_t pa, int perm) {
    if (PGROUNDDOWN(va) != va || PGROUNDDOWN(pa) != pa) return -1;
    pte_t *pte = walk(pagetable, va, 1);
    if (pte == 0) return -1;
    if (*pte & PTE_V) return -1;
    *pte = PA2PTE(pa) | perm | PTE_V;
    return 0;
}

pagetable_t create_pagetable() {
    pagetable_t pagetable = (pagetable_t)alloc_page();
    if (pagetable) {
        memset(pagetable, 0, PGSIZE);
    }
    return pagetable;
}


// --- 关键修正 #1: 修正 map_region 的签名和实现 ---
// 我们只使用 4KB 小页，移除大页逻辑以简化和确保正确性。
// 参数列表改为 (pt, va, pa, size, perm)，更清晰。
static void map_region(pagetable_t pt, uint64_t va, uint64_t pa, uint64_t size, int perm) {
    va = PGROUNDUP(va); // 从向上对齐的地址开始，确保不重叠
    pa = PGROUNDUP(pa);

    for (uint64_t i = 0; i < size; i += PGSIZE) {
        if (map_page(pt, va + i, pa + i, perm) != 0) {
            // 内核初始化失败，停机
            while(1);
        }
    }
}


// --- 关键修正 #2: 用正确的参数调用 map_region ---
void kvminit() {
    kernel_pagetable = create_pagetable();

    // 1. 映射 UART 设备
    printf("Mapping UART...\n");
    map_page(kernel_pagetable, UARTO, UARTO, PTE_R | PTE_W);

    // 2. 映射 VirtIO MMIO 区域 (0x10001000-0x10008000)
    printf("Mapping VirtIO MMIO...\n");
    for (uint64_t addr = 0x10001000; addr < 0x10008000; addr += PGSIZE) {
        map_page(kernel_pagetable, addr, addr, PTE_R | PTE_W);
    }

    // 3. 映射内核代码段 (.text)
    printf("Mapping Kernel Code (.text)...\n");
    map_region(kernel_pagetable,
               KERNBASE,                           // va_start
               KERNBASE,                           // pa_start
               (uint64_t)etext - KERNBASE,         // size
               PTE_R | PTE_X);

    // 4. 映射内核数据段和剩余内存
    printf("Mapping Kernel Data and remaining RAM...\n");
    map_region(kernel_pagetable,
               (uint64_t)etext,                    // va_start
               (uint64_t)etext,                    // pa_start
               (uint64_t)PHYSTOP - (uint64_t)etext, // size
               PTE_R | PTE_W);

    // 5. 映射 trampoline 页面
    // trampoline 代码在内核中，我们将它映射到 TRAMPOLINE 虚拟地址
    // 这样在内核和用户页表中都可以访问同一物理页
    // 链接脚本已经将trampoline放在页边界上，所以地址应该已经是页对齐的
    //
    // 在内核页表中映射trampoline页面
    // 重要：在内核页表中，TRAMPOLINE不应该有U标志！
    // 原因：S-mode不能执行（fetch instruction）标记为U的页面，即使设置了sstatus.SUM
    // sstatus.SUM只允许S-mode load/store访问U页面，但不允许执行
    printf("Mapping trampoline page at VA 0x%lx...\n", TRAMPOLINE);
    extern char trampoline[];  // 来自链接脚本，指向内核中的物理地址
    uint64_t trampoline_pa = (uint64_t)trampoline;
    printf("Trampoline physical address: 0x%lx\n", trampoline_pa);
    // 内核页表中：只设置R和X标志，不设置U标志
    if (map_page(kernel_pagetable, TRAMPOLINE, trampoline_pa, PTE_R | PTE_X) != 0) {
        printf("ERROR: Failed to map trampoline page!\n");
        while(1);
    }
    printf("Trampoline mapped successfully\n");
}


// kvminithart 函数 (你的版本是正确的，保持不变)
void kvminithart() {
    uint64_t satp = MAKE_SATP(kernel_pagetable);
    asm volatile("csrw satp, %0" : : "r"(satp));
    asm volatile("sfence.vma");
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

// 在 vm.c 文件末尾添加这个函数

void test_page_table_remap() {
    printf("\n--- Starting Page Table Remap & Perm Test ---\n");

    pagetable_t pt = create_pagetable();
    assert(pt != 0);

    uint64_t va = 0x200000;
    void* pa_ptr = alloc_page();
    assert(pa_ptr != 0);
    uint64_t pa = (uint64_t)pa_ptr;

    // 1. 正常映射
    printf("   Testing initial mapping...\n");
    assert(map_page(pt, va, pa, PTE_R | PTE_W) == 0);
    
    // 2. 重复映射测试
    printf("   Testing remap failure...\n");
    // 尝试用不同权限再次映射同一个VA，应该失败
    assert(map_page(pt, va, pa, PTE_R | PTE_X) == -1); 
    printf("   Remap correctly failed as expected.\n");

    // 3. 权限验证
    printf("   Verifying permissions...\n");
    pte_t *pte = walk(pt, va, 0);
    assert(pte != 0 && (*pte & PTE_V));
    // 检查权限是否与第一次映射时设置的一致
    assert((*pte & (PTE_R | PTE_W)) == (PTE_R | PTE_W));
    assert(!(*pte & PTE_X));
    printf("   Permissions are correctly set.\n");

    // 清理
    // 在实际的 unmap 函数实现前，我们手动清除 PTE
    *pte = 0;
    free_page(pa_ptr);
    // 注意：walk 分配的中间页表没有被释放，这里有轻微的内存泄漏，
    // 但对于测试来说是可接受的。
    free_page(pt);

    printf("--- Page Table Remap & Perm Test Finished ---\n");
}

// 在 vm.c 文件末尾添加这个函数，它需要访问 static 的 kernel_pagetable

void test_kernel_mapping() {
    printf("\n--- Starting Kernel Mapping Verification Test ---\n");

    pte_t *pte;
    uint64_t pa;

    // 1. 检查内核代码段的第一个页面
    printf("   Verifying kernel code mapping (.text start)...\n");
    pte = walk(kernel_pagetable, KERNBASE, 0);
    assert(pte != 0 && (*pte & PTE_V)); // 必须有效
    assert(*pte & PTE_X);               // 必须可执行
    assert(!(*pte & PTE_W));              // 必须不可写
    pa = PTE2PA(*pte);
    assert(pa == KERNBASE);             // 必须是恒等映射
    printf("   .text start mapping OK.\n");

    // 2. 检查内核数据段的一个页面 (etext 之后)
    printf("   Verifying kernel data mapping (.data start)...\n");
    uint64_t data_addr = PGROUNDUP((uint64_t)etext);
    pte = walk(kernel_pagetable, data_addr, 0);
    assert(pte != 0 && (*pte & PTE_V)); // 必须有效
    assert(!(*pte & PTE_X));              // 必须不可执行
    assert(*pte & PTE_W);               // 必须可写
    pa = PTE2PA(*pte);
    assert(pa == data_addr);            // 必须是恒等映射
    printf("   .data start mapping OK.\n");

    // 3. 检查 UART 的映射
    printf("   Verifying UART mapping...\n");
    pte = walk(kernel_pagetable, UARTO, 0);
    assert(pte != 0 && (*pte & PTE_V)); // 必须有效
    assert(*pte & PTE_W);               // 必须可写
    pa = PTE2PA(*pte);
    assert(pa == UARTO);                // 必须是恒等映射
    printf("   UART mapping OK.\n");

    printf("--- Kernel Mapping Verification Test Finished ---\n");
}



/**
 * @brief 创建一个新的、空的页表给用户进程使用。
 * @return 成功则返回页表地址，失败返回0。
 */
pagetable_t uvmcreate() {
    pagetable_t pagetable = (pagetable_t)alloc_page();
    if (pagetable == 0) {
        return 0;
    }
    memset(pagetable, 0, PGSIZE);

    // 将内核页表的映射复制到用户页表
    // 这样用户进程既可以访问用户空间，也可以访问内核空间（当trap时）
    extern pagetable_t kernel_pagetable;

    // 复制内核页表的高半部分映射 (地址 >= KERNBASE 以及 TRAMPOLINE/TRAPFRAME)
    // TRAMPOLINE 在 0x3ffffff000，其顶级页表索引是 255
    // 因此我们从索引 255 开始复制（而不是 256）
    for (int i = 255; i < 512; i++) {
        pagetable[i] = kernel_pagetable[i];
    }

    // 额外映射 trampoline 页面（如果还没有被复制）
    // TRAMPOLINE 在 0x3FFFFFE000，这应该已经被上面的循环复制了
    // 但为了明确性，我们验证一下
    extern char trampoline[];
    pte_t *pte = walk(pagetable, TRAMPOLINE, 0);
    if (pte == 0 || (*pte & PTE_V) == 0) {
        // 如果还没有映射，手动映射 (添加 PTE_U 标志 - xv6 style)
        if (map_page(pagetable, TRAMPOLINE, (uint64_t)trampoline, PTE_R | PTE_X | PTE_U) != 0) {
            free_page((void*)pagetable);
            return 0;
        }
    }

    return pagetable;
}

/**
 * @brief 递归释放一个页表的所有层级以及其映射的物理内存。
 * @param pagetable 要释放的页表。
 */
static void uvmfree_recursive(pagetable_t pagetable) {
    // 遍历页表的所有PTE
    for (int i = 0; i < 512; i++) {
        pte_t pte = pagetable[i];
        if ((pte & PTE_V) && (pte & (PTE_R | PTE_W | PTE_X)) == 0) {
            // 这是一个指向下一级页表的指针
            uint64_t child_pa = PTE2PA(pte);
            uvmfree_recursive((pagetable_t)child_pa);
        } else if (pte & PTE_V) {
            // 这是一个叶子节点，映射了一个物理页
            free_page((void*)PTE2PA(pte));
        }
    }
    // 释放当前级别的页表本身
    free_page((void*)pagetable);
}

/**
 * @brief 释放一个用户页表及其所有映射的物理内存。
 * @param pagetable 要释放的页表的根。
 * @param sz (未使用) 用户内存的大小。
 */
void uvmfree(pagetable_t pagetable, uint64_t sz) {
    if (pagetable == 0) {
        return;
    }
    // 简化的版本，只释放根页表。
    // 一个完整的实现需要递归遍历并释放所有级别的页表和物理页。
    // uvmfree_recursive(pagetable); // 使用这个函数来实现完整的释放
    free_page((void*)pagetable); // 暂时只释放根，以保持简单
}


/**
 * @brief 将父进程的用户内存复制到子进程。
 *        它会为子进程分配新的物理页，并复制内容。
 * @param old 父进程页表。
 * @param new 子进程页表。
 * @param sz (未使用) 要复制的内存大小。
 * @return 成功返回0，失败返回-1。
 */
int uvmcopy(pagetable_t old, pagetable_t new, uint64_t sz) {
    pte_t *pte;
    uint64_t pa, i;
    int flags;
    char *mem;

    // 这是一个简化的实现，只复制内核已映射的部分之下到sz大小的用户空间
    // 真实实现需要递归遍历
    for (i = 0; i < sz; i += PGSIZE) {
        if ((pte = walk(old, i, 0)) == 0) {
            continue; // 父进程没有映射这个页面，跳过
        }
        if ((*pte & PTE_V) == 0) {
            continue; // 页表项无效，跳过
        }
        pa = PTE2PA(*pte);
        flags = PTE_FLAGS(*pte);
        if ((mem = alloc_page()) == 0) {
            goto err; // 内存分配失败
        }
        // 复制页面内容
        memmove(mem, (char*)pa, PGSIZE);
        // 为子进程映射新页面
        if (map_page(new, i, (uint64_t)mem, flags) != 0) {
            free_page(mem);
            goto err;
        }
    }
    return 0;

err:
    // 如果出错，需要取消已经为子进程做的所有映射 (unmap)
    // uvmfree(new, 0); // 在一个完整的系统中需要这样做
    return -1;
}
