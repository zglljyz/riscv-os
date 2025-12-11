

#include <stdarg.h>
#include "types.h" 
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
    // 如果是有符号打印，且数字为负，记录
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


/**
 * @brief 核心的格式化输出函数，接收 va_list。
 *        这是所有printf类函数的底层实现。
 * @param fmt 格式字符串
 * @param ap  可变参数列表指针
 */
static void vprintf(const char *fmt, va_list ap)
{
    const char *p = fmt;
    int is_long = 0; // 标志位，用于判断是否有 'l' 修饰符 (64位)

    // 遍历整个格式字符串
    while (*p) {
        if (*p != '%') {
            uart_putc(*p);
            p++;
            continue; // 继续下一个字符
        }

        p++; // 跳过 '%'

        // 检查是否有 'l' 修饰符，用于支持 %ld 和 %lx
        if (*p == 'l') {
            is_long = 1;
            p++;
        }

        // 根据格式说明符进行处理
        switch (*p) {
            case 'd': { // 十进制整数
                // 使用三元运算符根据is_long标志决定取出的参数类型
                long long val = is_long ? va_arg(ap, long long) : va_arg(ap, int);
                print_number(val, 10, 1);
                break;
            }
            case 'u': { // 无符号十进制整数
                unsigned long long val = is_long ? va_arg(ap, unsigned long long) : va_arg(ap, unsigned int);
                print_number(val, 10, 0);
                break;
            }
            case 'x': { // 十六进制整数
                unsigned long long val = is_long ? va_arg(ap, unsigned long long) : va_arg(ap, unsigned int);
                print_number(val, 16, 0);
                break;
            }
            case 'p': { // 指针地址 (按64位十六进制处理)
                // 指针总是64位的，所以直接取出 unsigned long long
                unsigned long long val = va_arg(ap, unsigned long long);
                uart_puts("0x");
                print_number(val, 16, 0);
                break;
            }
            case 's': { // 字符串
                char *s = va_arg(ap, char *);
                if (s == NULL) { // 边界情况：处理空指针
                    uart_puts("(null)");
                } else {
                    uart_puts(s);
                }
                break;
            }
            case 'c': { // 单个字符
                // char 会被自动提升为 int
                char c = va_arg(ap, int);
                uart_putc(c);
                break;
            }
            case '%': { // 打印百分号本身
                uart_putc('%');
                break;
            }
            default: { // 未知的格式说明符
                // 原样打印，以便调试
                uart_putc('%');
                uart_putc(*p);
                break;
            }
        }

        is_long = 0; // 处理完一个格式符后，重置标志位
        p++;         // 指向下一个字符
    }
}

/**
 * @brief 公共的 printf 接口函数。
 *        它是一个简单的封装，负责初始化va_list并调用vprintf。
 */
void printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
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
