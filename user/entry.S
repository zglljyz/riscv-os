# 用户程序入口点
.section .text.start
.global _start
_start:
    # 调用main函数
    call main

    # main返回后，调用exit
    # a0已经包含main的返回值
    li a7, 2        # SYS_exit
    ecall

    # 不应该到达这里
1:
    j 1b
