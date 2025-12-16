
// QEMU virt机器将UART设备映射到物理地址0x10000000。
#define UART0 0x10000000L

// UART寄存器相对于基地址的偏移量。
#define THR 0 // (W) Transmitter Holding Register: 用于写入要发送的字符
#define RHR 0 // (R) Receiver Holding Register: 用于读取接收的字符
#define LSR 5 // (R) Line Status Register: 用于检查UART状态

// LSR寄存器中的位定义。
#define LSR_TX_IDLE (1 << 5) // Transmitter Holding Register is empty.
#define LSR_RX_READY (1 << 0) // Receiver Data Ready: 有数据可读

/**
 * @brief 向UART发送单个字符。
 *
 * 在写入新字符到THR之前，它会持续检查LSR寄存器的第5位(LSR_TX_IDLE)，
 * 直到UART准备好接收下一个要发送的字符。
 *
 * @param c 要发送的字符。
 */
void uart_putc(char c) {
    // 持续等待，直到发送保持寄存器为空。
    while ((*(volatile unsigned char *)(UART0 + LSR) & LSR_TX_IDLE) == 0)
        ;
    
    // 将字符写入发送保持寄存器。
    *(volatile unsigned char *)(UART0 + THR) = c;
}


void uart_puts(char *s) {
    while (*s) {
        uart_putc(*s);
        s++;
    }
}

/**
 * @brief 从UART读取单个字符（阻塞）
 *
 * 持续检查LSR寄存器的第0位(LSR_RX_READY)，
 * 直到有数据可读，然后返回该字符。
 *
 * @return 接收到的字符
 */
char uart_getc(void) {
    // 持续等待，直到接收数据就绪
    while ((*(volatile unsigned char *)(UART0 + LSR) & LSR_RX_READY) == 0)
        ;

    // 读取并返回接收到的字符
    return *(volatile unsigned char *)(UART0 + RHR);
}

