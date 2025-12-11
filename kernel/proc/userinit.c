// 用户初始化进程 - 创建第一个用户态进程并加载 shell 程序
#include "types.h"
#include "proc.h"
#include "vm.h"
#include "riscv.h"
#include "string.h"

// 外部声明
void printf(const char*, ...);
struct proc* allocproc(void);
pagetable_t uvmcreate(void);
int map_page(pagetable_t, uint64_t, uint64_t, int);
void* alloc_page(void);
void free_page(void*);
void uvmfree(pagetable_t, uint64_t);

/**
 * @brief 为用户进程分配和映射内存
 * @param pagetable 用户页表
 * @param va 虚拟地址
 * @param sz 要分配的字节数
 * @return 成功返回0，失败返回-1
 */
static int uvmalloc(pagetable_t pagetable, uint64_t va, uint64_t sz) {
    uint64_t a;

    // 按页对齐
    va = PGROUNDDOWN(va);
    uint64_t end = PGROUNDUP(va + sz);

    for (a = va; a < end; a += PGSIZE) {
        void *mem = alloc_page();
        if (mem == 0) {
            // 分配失败，释放已分配的内存
            for (uint64_t b = va; b < a; b += PGSIZE) {
                // 简化版：不释放（完整版应该释放）
            }
            return -1;
        }
        memset(mem, 0, PGSIZE);

        // 映射页面：用户可读、可写、可执行
        if (map_page(pagetable, a, (uint64_t)mem, PTE_R | PTE_W | PTE_X | PTE_U) != 0) {
            free_page(mem);
            return -1;
        }
    }

    return 0;
}

/**
 * @brief 将数据复制到用户空间
 * @param pagetable 用户页表
 * @param va 用户虚拟地址
 * @param src 源数据
 * @param len 数据长度
 * @return 成功返回0，失败返回-1
 */
static int copyout_to_user(pagetable_t pagetable, uint64_t va, char *src, uint64_t len) {
    uint64_t pa;
    uint64_t n;
    pte_t *pte;
    extern pte_t *walk(pagetable_t, uint64_t, int);

    while (len > 0) {
        // 获取虚拟地址对应的物理地址
        pte = walk(pagetable, va, 0);
        if (pte == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0) {
            return -1;
        }
        pa = PTE2PA(*pte) + (va & (PGSIZE - 1));

        // 计算当前页剩余空间
        n = PGSIZE - (va & (PGSIZE - 1));
        if (n > len) {
            n = len;
        }

        // 复制数据
        memmove((void*)pa, src, n);

        len -= n;
        src += n;
        va += n;
    }

    return 0;
}

/**
 * @brief 创建第一个用户进程并加载 shell 程序
 */
void userinit(void) {
    struct proc *p;

    printf("userinit: Creating first user process...\n");

    // 分配进程结构
    p = allocproc();
    if (p == 0) {
        printf("userinit: cannot allocate process\n");
        return;
    }

    // 创建用户页表
    p->pagetable = uvmcreate();
    if (p->pagetable == 0) {
        printf("userinit: cannot create page table\n");
        // Note: In a full implementation, we should clean up the process
        // For now, we'll just return (process remains allocated but unusable)
        return;
    }

    // 根据xv6,将trapframe映射到用户页表的TRAPFRAME虚拟地址
    // 这样trapframe在kernel和user页表中都可访问
    // 注意：xv6在用户页表中不设置PTE_U，因为TRAPFRAME是内核使用的
    extern int map_page(pagetable_t, uint64_t, uint64_t, int);
    if (map_page(p->pagetable, TRAPFRAME, (uint64_t)p->trapframe, PTE_R | PTE_W) != 0) {
        printf("userinit: cannot map trapframe\n");
        uvmfree(p->pagetable, 0);
        return;
    }

    // 分配用户内存 (4页: 代码+数据+栈)
    uint64_t sz = 4 * PGSIZE;  // 4页以容纳 shell 程序
    if (uvmalloc(p->pagetable, 0, sz) != 0) {
        printf("userinit: cannot allocate user memory\n");
        uvmfree(p->pagetable, sz);
        // Note: In a full implementation, we should clean up the process
        return;
    }
    p->sz = sz;

    // 加载shell程序
    extern unsigned char user_shell_bin[];
    extern unsigned int user_shell_bin_len;

    printf("userinit: Loading shell program (%d bytes) to user space at VA 0x0\n", user_shell_bin_len);

    if (copyout_to_user(p->pagetable, 0, (char*)user_shell_bin, user_shell_bin_len) != 0) {
        printf("userinit: cannot copy shell program\n");
        uvmfree(p->pagetable, sz);
        return;
    }

    printf("userinit: Shell program loaded successfully\n");

    // 验证映射
    extern pte_t *walk(pagetable_t, uint64_t, int);
    pte_t *pte;

    // 验证 TRAPFRAME 是否被正确映射
    pte = walk(p->pagetable, (uint64_t)TRAPFRAME, 0);
    if (pte && (*pte & PTE_V)) {
        printf("userinit: TRAPFRAME (0x%lx) is mapped to PA 0x%lx\n", (uint64_t)TRAPFRAME, PTE2PA(*pte));
        printf("userinit: TRAPFRAME PTE flags: V=%d R=%d W=%d X=%d U=%d\n",
               (*pte & PTE_V) ? 1 : 0,
               (*pte & PTE_R) ? 1 : 0,
               (*pte & PTE_W) ? 1 : 0,
               (*pte & PTE_X) ? 1 : 0,
               (*pte & PTE_U) ? 1 : 0);
    } else {
        printf("userinit: ERROR - TRAPFRAME is not mapped!\n");
    }

    // 验证 TRAMPOLINE 是否被正确映射
    pte = walk(p->pagetable, (uint64_t)TRAMPOLINE, 0);
    if (pte && (*pte & PTE_V)) {
        printf("userinit: TRAMPOLINE (0x%lx) is mapped to PA 0x%lx\n", (uint64_t)TRAMPOLINE, PTE2PA(*pte));
        printf("userinit: TRAMPOLINE PTE flags: V=%d R=%d W=%d X=%d U=%d\n",
               (*pte & PTE_V) ? 1 : 0,
               (*pte & PTE_R) ? 1 : 0,
               (*pte & PTE_W) ? 1 : 0,
               (*pte & PTE_X) ? 1 : 0,
               (*pte & PTE_U) ? 1 : 0);
    } else {
        printf("userinit: ERROR - TRAMPOLINE is not mapped!\n");
    }

    // 设置 trapframe
    p->trapframe->epc = 0x0;  // 用户程序入口点 (simple_initcode starts at 0x0)
    p->trapframe->sp = sz;  // 栈指针（第四页的结束地址，栈向下增长）

    // 初始化所有其他 trapframe 寄存器为 0
    p->trapframe->ra = 0;
    p->trapframe->gp = 0;
    p->trapframe->tp = 0;
    p->trapframe->t0 = p->trapframe->t1 = p->trapframe->t2 = 0;
    p->trapframe->s0 = p->trapframe->s1 = 0;
    p->trapframe->a0 = p->trapframe->a1 = p->trapframe->a2 = 0;
    p->trapframe->a3 = p->trapframe->a4 = p->trapframe->a5 = 0;
    p->trapframe->a6 = p->trapframe->a7 = 0;
    p->trapframe->s2 = p->trapframe->s3 = p->trapframe->s4 = p->trapframe->s5 = 0;
    p->trapframe->s6 = p->trapframe->s7 = p->trapframe->s8 = p->trapframe->s9 = 0;
    p->trapframe->s10 = p->trapframe->s11 = 0;
    p->trapframe->t3 = p->trapframe->t4 = p->trapframe->t5 = p->trapframe->t6 = 0;

    // 设置进程属性
    p->priority = PRIORITY_NORMAL;
    p->base_priority = PRIORITY_NORMAL;
    p->time_slice = 100;

    // 标记为可运行
    acquire(&ptable_lock);
    p->state = RUNNABLE;
    p->wait_start = get_time();
    release(&ptable_lock);

    printf("userinit: First user process (pid=%d) created successfully\n", p->pid);
}
