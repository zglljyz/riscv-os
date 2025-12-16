/* 指定目标架构 */
OUTPUT_ARCH("riscv")
/* 指定程序入口点为_entry*/
ENTRY(_entry)

SECTIONS
{
    /* 设置当前位置计数器为0x80200000 (OpenSBI之后) */
    . = 0x80200000;

    /* 代码段 */
    .text : {
        /* 确保entry.o在最前面，因为它是入口点 */
        kernel/boot/entry.o(.text)
        *(.text .text.*)          /* 所有其他代码 */
        . = ALIGN(4);
        PROVIDE(etext = .);       /* 定义代码段结束符号 */
    } 

    /* 只读数据段 */
    .rodata : {
        . = ALIGN(4);
        *(.rodata .rodata.*)      /* 只读数据 */
        *(.srodata .srodata.*)    /* 小只读数据 */
    } 

    /* 数据段 */
    .data : {
        . = ALIGN(4);
        *(.data .data.*)          /* 已初始化数据 */
        *(.sdata .sdata.*)        /* 小已初始化数据 */
    } 
    
    .stack (NOLOAD) : {
        . = ALIGN(16);
        PROVIDE(stack0 = .);
        . += 4096 * 4; 
        PROVIDE(stack_top = .);
    } 

    
    /* BSS段 */
    .bss : {
        . = ALIGN(4);
        PROVIDE(_bss_start = .);  /* BSS段开始符号 */
        *(.sbss .sbss.*)          /* 小未初始化数据 */
        *(.bss .bss.*)            /* 未初始化数据 */
        PROVIDE(_bss_end = .);    /* BSS段结束符号 */
    } 

    /* 内核结束位置符号 */
    PROVIDE(end = .);
    
    PROVIDE(PHYSTOP = 0x80000000 + 128 * 1024 * 1024);
}
