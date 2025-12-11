# 用户程序启动代码
# 用户程序的入口点，负责调用 main 函数并退出

.section .text
.global _start

_start:
    # 栈指针已经由内核设置好
    # a0 = argc, a1 = argv

    # 调用 main 函数
    call main

    # main 返回后，调用 exit 系统调用
    # a0 中已经包含了 main 的返回值
    call exit

    # 不应该执行到这里
    j .
