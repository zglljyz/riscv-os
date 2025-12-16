
// QEMU virt机器将UART设备映射到物理地址0x10000000。
#define UART0 0x10000000L

// UART寄存器相对于基地址的偏移量。
#define THR 0 // (W) Transmitter Holding Register: 用于写入要发送的字符
#define LSR 5 // (R) Line Status Register: 用于检查UART状态

// LSR寄存器中的位定义。
#define LSR_TX_IDLE (1 << 5) // Transmitter Holding Register is empty.

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

