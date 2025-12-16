#include "riscv.h"
#include "pmm.h"
#include "types.h"
#include "string.h"
void printf(const char *fmt, ...);

extern char end[];
extern char PHYSTOP[];
struct run { struct run *next; };
static struct run *freelist;


#define CACHE_SIZE 8 // 每个缓存池的大小
static void* page_cache[CACHE_SIZE];
static int cache_count = 0;

// 从全局空闲链表填充本地缓存
static void refill_cache() {
    printf("   [PMM Cache: Refilling...]\n");
    while (cache_count < CACHE_SIZE) {
        struct run *r = freelist;
        if (r) {
            freelist = r->next;
            page_cache[cache_count++] = (void*)r;
        } else {
            break; // 全局内存池也空了
        }
    }
}

// 清空本地缓存到全局空闲链表
static void flush_cache() {
    printf("   [PMM Cache: Flushing...]\n");
    while (cache_count > 0) {
        void* pa = page_cache[--cache_count];
        
        struct run *r = (struct run*)pa;
        r->next = freelist;
        freelist = r;
    }
}


void pmm_init()
{
    // 构建全局 freelist
    struct run *head = NULL;
    void *pa_start = (void*)PGROUNDUP((uint64_t)end);
    for (void *p = pa_start; p + PGSIZE <= (void*)PHYSTOP; p += PGSIZE) {
        struct run *r = (struct run*)p;
        r->next = head;
        head = r;
    }
    freelist = head;
    
    // 初始化完成后，填充一次缓存
    refill_cache(); 
}

void free_page(void *pa)
{
    if (pa == 0 || (uint64_t)pa % PGSIZE != 0) return;

    // 如果缓存满了，就先清空缓存到全局链表
    if (cache_count >= CACHE_SIZE) {
        flush_cache();
    }

    // 将页面内容清零
    // 必须在放入缓存前进行清理。
    memset(pa, 1, PGSIZE);

    // 将干净的页面放入缓存
    page_cache[cache_count++] = pa;
}


void *alloc_page() {
    // 如果缓存是空的，先填充缓存
    if (cache_count == 0) {
        refill_cache();
        // 填充后仍然是空的，说明内存耗尽
        if (cache_count == 0) {
            return 0;
        }
    }

    // 从缓存中取出一个页面
    void *pa = page_cache[--cache_count];
    return pa;
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
