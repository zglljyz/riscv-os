

#include <stdarg.h>

// 调用 uart.c 中的函数，在此声明它们。
void uart_putc(char c);
void uart_puts(char *s);

// 用于进制转换的字符查找表
static char digits[] = "0123456789abcdef";

/**
 * @brief 打印一个整数的核心函数。
 * 
 * @param num       要打印的数字。使用long long类型以避免INT_MIN的溢出问题。
 * @param base      要转换的进制（例如：10代表十进制，16代表十六进制）。
 * @param is_signed 布尔值，如果为true，则将num视为有符号数处理（会打印负号）。
 */
static void print_number(long long num, int base, int is_signed)
{
    // 一个临时缓冲区，用于存放转换后的数字字符。
    // 20个字符足以存放64位整数在二进制下的所有位和一个负号。
    char buf[20];
    int i = 0;
    unsigned long long n; // 使用无符号类型来处理数字，简化逻辑

    // --- 步骤1: 处理符号，并将数字转换为正数 ---
    // 如果是有符号打印，且数字为负，我们记录下这个事实，
    // 然后将数字转换为其正数形式。
    // 之后的所有转换逻辑都只需要处理正数。
    int sign = 0;
    if (is_signed && num < 0) {
        sign = 1;
        n = -num;
    } else {
        n = num;
    }

    // --- 步骤2: 核心转换循环 ---
    // 这个do-while循环通过“取模”和“除法”来获取数字的每一位。
    // 它生成的数字顺序是反的（从个位开始）。
    // 使用do-while可以正确处理 num 为 0 的情况。
    do {
        buf[i++] = digits[n % base];
        n /= base;
    } while (n > 0);

    // 如果原始数字是负数，在缓冲区的末尾添加负号
    if (sign) {
        buf[i++] = '-';
    }

    // --- 步骤3: 按正确顺序打印 ---
    // 因为buf中的字符是反的，所以从后往前遍历buf，
    // 逐个字符地调用uart_putc()，这样输出的顺序就是正确的了。
    while (--i >= 0) {
        uart_putc(buf[i]);
    }
}


void printf(const char *fmt, ...)
{
    va_list ap; // 定义一个指向可变参数列表的指针
    va_start(ap, fmt); // 初始化 ap，使其指向第一个可变参数

    const char *p = fmt;
    while (*p) {
        if (*p != '%') {
            // 如果不是格式说明符，直接打印字符
            uart_putc(*p);
        } else {
            // 跳过 '%'
            p++;
            // 根据 '%' 后面的字符进行处理
            switch (*p) {
                case 'd': { // 十进制整数
                    int i = va_arg(ap, int);
                    print_number(i, 10, 1);
                    break;
                }
                case 'x': { // 十六进制整数
                    int i = va_arg(ap, int);
                    print_number(i, 16, 0);
                    break;
                }
                case 'p': { // 指针
                    // 指针在64位架构下是64位长，用 long long 来接收
                    unsigned long long i = va_arg(ap, unsigned long long);
                    uart_puts("0x");
                    print_number(i, 16, 0);
                    break;
                }
                case 's': { // 字符串
                    char *s = va_arg(ap, char *);
                    if (s == 0) { // 处理空指针的边界情况
                        uart_puts("(null)");
                    } else {
                        uart_puts(s);
                    }
                    break;
                }
                case 'c': { // 字符
                    // char 类型在作为可变参数传递时会被提升为 int
                    char c = va_arg(ap, int);
                    uart_putc(c);
                    break;
                }
                case '%': { // 打印一个 '%'
                    uart_putc('%');
                    break;
                }
                default: { // 未知格式符
                    // 遵从我们的设计决策：原样打印
                    uart_putc('%');
                    uart_putc(*p);
                    break;
                }
            }
        }
        p++; // 处理下一个字符
    }

    va_end(ap); // 清理可变参数列表
}

/**
 * @brief 使用ANSI转义序列来清空屏幕。
 */
void clear_screen(void)
{
    // \033[2J: 清除整个屏幕
    // \033[H:  将光标移动到左上角 (第一行,第一列)
    printf("\033[2J\033[H");
}

/**
 * @brief (扩展功能1) 将光标移动到指定位置。
 *        (1,1) 是屏幕左上角。
 * @param x 列号 (column)
 * @param y 行号 (row)
 */
void goto_xy(int x, int y)
{
    // ANSI序列: \033[<行号>;<列号>H
    printf("\033[%d;%dH", y, x);
}

/**
 * @brief (扩展功能2) 清除光标当前所在的整行。
 */
void clear_line(void)
{
    // ANSI序列: \033[2K 清除整行
    // \r:         回车，将光标移到行首, 以便在清除后立即打印
    printf("\033[2K\r");
}
