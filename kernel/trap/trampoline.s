# -----------------------------------------------------------------
# kernel/trap/trampoline.s
#
# Trampoline page - mapped at the same VA in both kernel and user page tables.
# This allows safe page table switching during trap entry/exit.
# Based on xv6's trampoline.S
# -----------------------------------------------------------------

.section .text
.globl trampoline
trampoline:
.align 4
.globl uservec
uservec:
    # 完全按照xv6的uservec实现
    # 此时trap刚发生,PC已经指向这里,使用的是用户页表

    # 保存用户a0到sscratch，这样a0可以用来访问TRAPFRAME
    csrw sscratch, a0

    # 加载TRAPFRAME地址(在用户页表中映射)
    li a0, 0x3fffffe000

    # 保存用户寄存器到 trapframe - xv6 offsets!
    sd ra, 40(a0)
    sd sp, 48(a0)
    sd gp, 56(a0)
    sd tp, 64(a0)
    sd t0, 72(a0)
    sd t1, 80(a0)
    sd t2, 88(a0)
    sd s0, 96(a0)
    sd s1, 104(a0)
    sd a1, 120(a0)
    sd a2, 128(a0)
    sd a3, 136(a0)
    sd a4, 144(a0)
    sd a5, 152(a0)
    sd a6, 160(a0)
    sd a7, 168(a0)
    sd s2, 176(a0)
    sd s3, 184(a0)
    sd s4, 192(a0)
    sd s5, 200(a0)
    sd s6, 208(a0)
    sd s7, 216(a0)
    sd s8, 224(a0)
    sd s9, 232(a0)
    sd s10, 240(a0)
    sd s11, 248(a0)
    sd t3, 256(a0)
    sd t4, 264(a0)
    sd t5, 272(a0)
    sd t6, 280(a0)

    # 保存用户的 a0（从 sscratch 中取回）
    csrr t0, sscratch
    sd t0, 112(a0)

    # 从 trapframe 中读取内核栈指针、内核页表、内核 trap 处理函数地址
    # xv6 layout: kernel_satp=0, kernel_sp=8, kernel_trap=16
    ld sp, 8(a0)       # kernel_sp
    ld t1, 0(a0)       # kernel_satp
    ld t0, 16(a0)      # kernel_trap function pointer

    # 切换到内核页表
    sfence.vma zero, zero
    csrw satp, t1
    sfence.vma zero, zero

    # 跳转到 usertrap()
    jr t0

.globl userret
userret:
    # 完全按xv6方式: a0包含user satp值
    # 此时我们在S-mode，使用kernel page table
    # 目标：切换到user page table，恢复用户寄存器，sret返回U-mode

    # 1. 切换到用户页表(a0中已经是user_satp值)
    sfence.vma zero, zero
    csrw satp, a0
    sfence.vma zero, zero

    # 2. 加载TRAPFRAME地址到a0
    li a0, 0x3fffffe000  # TRAPFRAME address

    # 3. 从trapframe恢复所有用户寄存器 - xv6方式，不包括a0
    ld ra, 40(a0)
    ld sp, 48(a0)
    ld gp, 56(a0)
    ld tp, 64(a0)
    ld t0, 72(a0)
    ld t1, 80(a0)
    ld t2, 88(a0)
    ld s0, 96(a0)
    ld s1, 104(a0)
    ld a1, 120(a0)
    ld a2, 128(a0)
    ld a3, 136(a0)
    ld a4, 144(a0)
    ld a5, 152(a0)
    ld a6, 160(a0)
    ld a7, 168(a0)
    ld s2, 176(a0)
    ld s3, 184(a0)
    ld s4, 192(a0)
    ld s5, 200(a0)
    ld s6, 208(a0)
    ld s7, 216(a0)
    ld s8, 224(a0)
    ld s9, 232(a0)
    ld s10, 240(a0)
    ld s11, 248(a0)
    ld t3, 256(a0)
    ld t4, 264(a0)
    ld t5, 272(a0)
    ld t6, 280(a0)

    # 4. 最后恢复a0
    ld a0, 112(a0)

    # 5. 返回到用户模式
    # sret会:
    #  - 将PC设置为sepc (之前在forkret中设置的)
    #  - 将特权级设置为sstatus.SPP (0=U-mode)
    #  - 将中断使能设置为sstatus.SPIE
    sret

.globl trampoline_end
trampoline_end:
