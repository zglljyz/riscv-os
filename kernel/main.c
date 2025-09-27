// --- 前向声明 ---
void printf(const char *fmt, ...);
void clear_screen(void); 
void goto_xy(int x, int y);      
void clear_line(void);      
// 一个简单的延时函数，通过CPU空转来实现
void delay(int count) {
    // volatile 关键字防止编译器优化掉这个循环
    for (volatile int i = 0; i < count; i++);
}

/**
 * @brief 内核的C语言入口函数
 * 
 * 这个函数由 kernel/boot/entry.S 中的汇编代码调用。
 */
void main(void) {
    printf("Hello OS, this is a real printf!\n\n");

    printf("--- Testing printf ---\n");
    
    // 测试 %d (整数)
    printf("Integer: %d\n", 2147483647);
    printf("Negative: %d\n", -2147483648);
    printf("Zero: %d\n", 0);

    // 测试 %x (十六进制)
    printf("Hex: 0x%x\n", 0xABCD);

    // 测试 %p (指针)
    int a = 10;
    printf("Pointer: %p\n", &a);
    printf("NULL Pointer: %p\n", (void*)0);

    // 测试 %s (字符串)
    printf("String: %s\n", "Hello World");
    printf("NULL String: %s\n", (char*)0);

    // 测试 %c (字符)
    printf("Character: %c\n", 'A');

    // 测试 %% (百分号)
    printf("Percent: 100%%\n");
    
    // 测试未知格式符
    printf("Unknown specifier: %z should be printed as is.\n");

    printf("\n--- All tests passed! ---\n");

    printf("Hello! This text will be cleared in a moment...\n");
    printf("Line 2...\n");
    printf("Line 3...\n");

    // 等待大约一两秒，让我们能看到上面的文字
    delay(10000000); 

    // 调用清屏函数
    clear_screen();

    printf("The screen is now clean!\n");
    printf("And we are printing from the top-left corner.\n");

    // 2. 测试 goto_xy: 在屏幕不同位置打印
    printf("Step 1: Testing goto_xy()\n");
    goto_xy(10, 5); // 移动到第5行，第10列
    printf("--> Here at (10, 5)");
    
    goto_xy(20, 10); // 移动到第10行，第20列
    printf("--> And now at (20, 10)");
    
    goto_xy(1, 15); // 移动到第15行，行首
    printf("Moving to a new line (1, 15) before clearing test.\n");
    delay(20000000);

    // 3. 测试 clear_line
    printf("\nStep 2: Testing clear_line()\n");
    
    goto_xy(1, 18);
    printf("This is a very long line that we are about to clear...");
    
    // 等待，让我们能看到这行字
    delay(20000000);
    
    // 清除刚刚打印的那一行
    goto_xy(1, 18); // 先把光标移回到要清除的那一行的开头
    clear_line();
    printf("The line above was just cleared!\n");


    goto_xy(1, 22);
    printf("\n\nAll tests completed!\n");
    // 死循环
    while(1);
}
