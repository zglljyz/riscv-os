
#include "riscv.h"
#include "pmm.h"
#include "types.h"
#include "string.h"

void printf(const char *fmt, ...);
extern char end[];
extern char PHYSTOP[];

// 局部 assert 定义
#define assert(expr) if (!(expr)) { printf("Assertion failed\n"); while(1); }

struct run {
    struct run *next;
};
static struct run *freelist;

void pmm_init() {
    freelist = NULL; // 明确初始化
    void *p = (void*)PGROUNDUP((uint64_t)end);
    for (; p + PGSIZE <= (void*)PHYSTOP; p += PGSIZE) {
        free_page(p);
    }
}

void free_page(void *pa) {
    if (((uint64_t)pa % PGSIZE) != 0 || (char*)pa < end || (char*)pa >= (char*)PHYSTOP) {
         printf("PANIC: invalid free_page\n"); while(1);
    }
    memset(pa, 1, PGSIZE); // 写入垃圾数据，暴露 use-after-free
    struct run *r = (struct run*)pa;
    r->next = freelist;
    freelist = r;
}

void* alloc_page(void) {
    struct run *r = freelist;
    if (r) {
        freelist = r->next;
    }
    if (r) {
        memset((char*)r, 5, PGSIZE); // 写入另一种垃圾数据，暴露问题
    }
    return (void*)r;
}



//PMM 单元测试函数

void test_pmm() {
    printf("\n--- Starting PMM Unit Tests ---\n");
    
    void *p1 = alloc_page();
    printf("   Allocated page 1 at: %p\n", p1);
    void *p2 = alloc_page();
    printf("   Allocated page 2 at: %p\n", p2);

    if (p1 == 0 || p2 == 0) {
        printf("   Error: alloc_page() returned NULL!\n");
        return;
    }
    if (p1 == p2) {
        printf("   Error: alloc_page() returned the same page twice!\n");
        return;
    }

    free_page(p1);
    printf("   Freed page 1.\n");
    
    void *p3 = alloc_page();
    printf("   Allocated page 3 at: %p\n", p3);
    if (p1 == p3) {
        printf("   Success: Re-allocated the freed page correctly.\n");
    } else {
        printf("   Warning: Did not re-allocate the exact same page. (This is OK)\n");
    }
    
    free_page(p2);
    free_page(p3);
    printf("--- PMM Unit Tests Finished ---\n");
}

#define PMM_TEST_PAGES 1024
static void* pages[PMM_TEST_PAGES];

void test_pmm_consistency() {
    printf("\n--- Starting PMM Consistency Test ---\n");
    
    // 1. 大量分配测试
    printf("   Attempting to allocate %d pages...\n", PMM_TEST_PAGES);
    int allocated_count = 0;
    for (int i = 0; i < PMM_TEST_PAGES; ++i) {
        pages[i] = alloc_page();
        if (pages[i] == NULL) {
            printf("   PMM exhausted after allocating %d pages. This is expected.\n", i);
            break;
        }
        allocated_count++;
    }
    printf("   Successfully allocated %d pages.\n", allocated_count);

    // 2. 连续性检查 (可选，因为链表分配不保证连续)
    //    但我们可以检查地址是否都在有效范围内，并且没有重复
    for (int i = 0; i < allocated_count; ++i) {
        assert((uint64_t)pages[i] % PGSIZE == 0); // 页对齐
        assert((char*)pages[i] >= end && (char*)pages[i] < (char*)PHYSTOP); // 范围
        for (int j = i + 1; j < allocated_count; ++j) {
            assert(pages[i] != pages[j]); // 无重复
        }
    }
    printf("   Address validation passed (alignment, range, uniqueness).\n");

    // 3. 大量释放测试
    printf("   Freeing all %d allocated pages...\n", allocated_count);
    for (int i = 0; i < allocated_count; ++i) {
        free_page(pages[i]);
    }
    printf("   All pages freed.\n");

    // 4. 再次分配，验证内存已回收
    printf("   Attempting to re-allocate one page...\n");
    void *p = alloc_page();
    assert(p != NULL);
    printf("   Re-allocation successful, got page at %p.\n", p);
    free_page(p);
    
    printf("--- PMM Consistency Test Finished ---\n");
}

