.section .text
.global _entry

_entry:
        # 设置C语言的栈指针
        la sp, stack_top

        # 清零BSS段
        call clear_bss

        # 跳转到C主函数main
        call main

spin:
        j spin

# BSS清零函数
clear_bss:
        la a0, _bss_start
        la a1, _bss_end
bss_clear_loop:
        beq a0, a1, bss_clear_done
        sw zero, 0(a0)
        addi a0, a0, 4
        j bss_clear_loop
bss_clear_done:
        ret

# 在同一个文件中定义栈空间
.section .bss
.align 16        # 16字节对齐，满足RISC-V ABI要求
stack0:
        .skip 4096 * 4    # 分配4KB栈空间
stack_top:                 # 栈顶标签，便于直接引用
