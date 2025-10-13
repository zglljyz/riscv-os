
// 基础I/O函数
void printf(const char *fmt, ...);

// 实验二扩展功能
void clear_screen(void);
void goto_xy(int x, int y);
void clear_line(void);

// 实验三PMM功能
#include "types.h"
#include "riscv.h" // 包含PGSIZE等定义
#include "pmm.h" 
#include "vm.h" 

// --- 辅助函数 ---
void delay(int count) {
    for (volatile int i = 0; i < count; i++);
}


//实验二 测试函数

void test_lab2(void)
{
    printf("\n\n--- Starting Lab 2 (printf and console control) Tests ---\n");
    delay(10000000); // 暂停

    // --- 1. 清屏功能测试 ---
    clear_screen();
    printf("1. Screen Cleared.\n");
    delay(10000000);

    // --- 2. printf 核心格式化功能测试 ---
    printf("\n2. Testing printf formatting:\n");
    
    // 测试 %d (整数), 包括边界值
    printf("   - Integer (Positive): %d\n", 2147483647);
    printf("   - Integer (Negative/Min): %d\n", -2147483648);
    printf("   - Integer (Zero): %d\n", 0);

    // 测试 %x (十六进制)
    printf("   - Hex: 0x%x\n", 0xABCD);

    // 测试 %p (指针)
    int a = 10;
    printf("   - Pointer: %p\n", &a);
    printf("   - NULL Pointer: %p\n", (void*)0);

    // 测试 %s (字符串), 包括边界值
    printf("   - String: %s\n", "Hello World");
    printf("   - NULL String: %s\n", (char*)0);

    // 测试 %c, %% 和未知格式符
    printf("   - Character: %c | Percent: 100%% | Unknown: %z\n", 'A');
    delay(20000000);

    // --- 3. 光标定位 (goto_xy) 功能测试 ---
    printf("\n3. Testing cursor positioning (goto_xy):\n");
    goto_xy(20, 15);
    printf("--> Text at (20, 15)");
    delay(10000000);
    
    goto_xy(5, 18);
    printf("--> And other text at (5, 18)");
    delay(10000000);

    // --- 4. 行清除 (clear_line) 功能测试 ---
    goto_xy(1, 20); // 移动到一个新行
    printf("\n4. Testing line clearing (clear_line):\n");
    printf("   This is a long line that will be erased in 2 seconds...");
    delay(20000000);
    
    // 清除刚刚打印的一行
    goto_xy(1, 21); 
    clear_line();
    printf("   The line above was just cleared!\n");
    delay(10000000);

    printf("\n--- Lab 2 All tests completed! ---\n");
}


// 主函数

void main(void) {


    // 调用实验二的测试函数
    // test_lab2();

    // 1. 初始化物理内存管理器
    pmm_init();
    printf("PMM Initialized.\n");
    // 运行PMM的单元测试
    test_pmm();

    // 2. 运行页表管理器的单元测试 (在开启分页前)
    test_pagetable();

    // 3. 创建内核页表
    kvminit();
    printf("\nKernel Page Table Created.\n");
    
    // 4. 开启分页
    kvminithart();
    
    printf("\nCongratulations! Virtual memory is enabled.\n");
    printf("All initial tests are done.\n");
    
    // 死循环
    while(1);
}
