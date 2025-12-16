// main.c - 实验1版本
void uart_puts(char *s);
void main(void) {

    // 输出 Hello OS
    uart_puts("Hello OS\n");
    
    // 测试一下输出多行，确保功能正常
    uart_puts("Lab 1: Successful!\n");

    // 死循环
    while(1);
}
