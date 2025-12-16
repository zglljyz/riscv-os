
// 基础I/O函数
void printf(const char *fmt, ...);

// 实验三PMM功能
#include "types.h"
#include "riscv.h" // 包含PGSIZE等定义
#include "pmm.h" 
#include "vm.h" 
// 主函数
void main(void) {



    // 1. 初始化物理内存管理器
    pmm_init();
    printf("PMM Initialized.\n");
    
    // 运行PMM的单元测试
    test_pmm();

    // 2. 运行页表管理器的单元测试
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
