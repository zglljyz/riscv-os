.section .text
.global _entry

_entry:
    # -----------------------------------------------------------------
    # S-mode entry: OpenSBI已经完成M-mode设置并切换到S-mode
    # 我们的内核直接从S-mode开始运行
    # -----------------------------------------------------------------

    # 1. 设置栈指针
    la sp, stack_top

    # 2. 清零 BSS 段
    call clear_bss

    # 3. 跳转到 C 主函数 main()
    #    main() 函数将负责初始化S-mode的中断(stvec, sie, sstatus)
    #    并进行第一次的SBI调用来设置时钟
    call main

spin:
    j spin

# BSS 清零函数实现
clear_bss:
    la a0, _bss_start
    la a1, _bss_end
bss_clear_loop:
    beq a0, a1, bss_clear_done
    sd zero, 0(a0)
    addi a0, a0, 8
    j bss_clear_loop
bss_clear_done:
    ret


