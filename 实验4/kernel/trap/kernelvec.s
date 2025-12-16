# -----------------------------------------------------------------
# kernel/kernelvec.s
#
# RISC-V S-mode (Supervisor mode) 陷阱向量入口
# 当任何中断或异常在S-mode下发生时，硬件会关闭中断并将PC跳转到这里。
# -----------------------------------------------------------------

.section .text
.global kernelvec  # 声明 kernelvec 为全局符号，供 C 代码引用
.align 2           # 确保代码按4字节对齐

#
# kernelvec() - 所有S-mode陷阱的统一入口点
#
kernelvec:
    # -------------------------------------------------------------
    # 步骤 1: 首先检查陷阱来源，决定如何处理栈
    # -------------------------------------------------------------
    # 读取 sstatus 寄存器，检查 SPP 位 (bit 8)。
    # 如果 SPP == 1, 说明陷阱来自 S-mode (内核态)。
    # 如果 SPP == 0, 说明陷阱来自 U-mode (用户态)。
    csrr t0, sstatus
    li t1, 1 << 8       # SSTATUS_SPP 位的掩码
    and t0, t0, t1      # 提取 SPP 位

    # 如果 SPP 位不为 0 (即为 1), 跳转到内核陷阱处理路径。
    bnez t0, kernel_to_kernel_trap

# ---- 用户态到内核态陷阱处理路径 ----
user_to_kernel_trap:
    # 陷阱来自用户态，需要切换到内核栈
    # 这是处理用户态陷阱最关键的一步。
    # csrrw 指令原子地交换 sp 和 sscratch 的值。
    # 这样，sp 立即指向了内核内存，我们就可以安全地保存寄存器了。
    csrrw sp, sscratch, sp

    # 保存所有通用寄存器到陷阱帧中
    sd ra, 0(sp)      # 保存返回地址 (ra, x1)
    csrr t0, sscratch
    sd t0, 8(sp)      # 保存原始栈指针 (sp, x2)
    sd gp, 16(sp)     # 保存全局指针 (gp, x3)
    sd tp, 24(sp)     # 保存线程指针 (tp, x4)
    sd t0, 32(sp)     # 保存临时寄存器 t0 (x5)
    sd t1, 40(sp)     # 保存临时寄存器 t1 (x6)
    sd t2, 48(sp)     # 保存临时寄存器 t2 (x7)
    sd s0, 56(sp)     # 保存保存寄存器 s0 (x8)
    sd s1, 64(sp)     # 保存保存寄存器 s1 (x9)
    sd a0, 80(sp)     # 保存参数/返回值寄存器 a0 (x10)
    sd a1, 88(sp)     # 保存参数/返回值寄存器 a1 (x11)
    sd a2, 96(sp)     # 保存参数寄存器 a2 (x12)
    sd a3, 104(sp)    # 保存参数寄存器 a3 (x13)
    sd a4, 112(sp)    # 保存参数寄存器 a4 (x14)
    sd a5, 120(sp)    # 保存参数寄存器 a5 (x15)
    sd a6, 128(sp)    # 保存参数寄存器 a6 (x16)
    sd a7, 136(sp)    # 保存参数寄存器 a7 (x17)
    sd s2, 144(sp)    # 保存保存寄存器 s2 (x18)
    sd s3, 152(sp)    # 保存保存寄存器 s3 (x19)
    sd s4, 160(sp)    # 保存保存寄存器 s4 (x20)
    sd s5, 168(sp)    # 保存保存寄存器 s5 (x21)
    sd s6, 176(sp)    # 保存保存寄存器 s6 (x22)
    sd s7, 184(sp)    # 保存保存寄存器 s7 (x23)
    sd s8, 192(sp)    # 保存保存寄存器 s8 (x24)
    sd s9, 200(sp)    # 保存保存寄存器 s9 (x25)
    sd s10, 208(sp)   # 保存保存寄存器 s10 (x26)
    sd s11, 216(sp)   # 保存保存寄存器 s11 (x27)
    sd t3, 224(sp)    # 保存临时寄存器 t3 (x28)
    sd t4, 232(sp)    # 保存临时寄存器 t4 (x29)
    sd t5, 240(sp)    # 保存临时寄存器 t5 (x30)
    sd t6, 248(sp)    # 保存临时寄存器 t6 (x31)

    # 调用 C 函数 usertrap() 进行处理。
    call usertrap
    # usertrap() 函数的职责是处理系统调用、缺页等，
    # 并且在处理完成后，必须调用 usertrapret()。
    # usertrapret() 会调用下面的 userret，所以 usertrap 不会返回到这里。
    # 我们在这里加一个循环以防万一。
    j hang

# ---- 内核态到内核态陷阱处理路径 ----
kernel_to_kernel_trap:
    # 陷阱来自内核态，我们已经在使用内核栈，不需要切换
    # 在当前栈上为寄存器保存预留空间
    addi sp, sp, -256  # 为保存寄存器分配栈空间

    # 保存所有通用寄存器
    sd ra, 0(sp)
    sd sp, 8(sp)       # 保存原始的内核栈指针
    sd gp, 16(sp)
    sd tp, 24(sp)
    sd t0, 32(sp)
    sd t1, 40(sp)
    sd t2, 48(sp)
    sd s0, 56(sp)
    sd s1, 64(sp)
    sd a0, 80(sp)
    sd a1, 88(sp)
    sd a2, 96(sp)
    sd a3, 104(sp)
    sd a4, 112(sp)
    sd a5, 120(sp)
    sd a6, 128(sp)
    sd a7, 136(sp)
    sd s2, 144(sp)
    sd s3, 152(sp)
    sd s4, 160(sp)
    sd s5, 168(sp)
    sd s6, 176(sp)
    sd s7, 184(sp)
    sd s8, 192(sp)
    sd s9, 200(sp)
    sd s10, 208(sp)
    sd s11, 216(sp)
    sd t3, 224(sp)
    sd t4, 232(sp)
    sd t5, 240(sp)
    sd t6, 248(sp)

    # 调用 C 函数 kerneltrap() 进行处理
    call kerneltrap
    # kerneltrap() 处理完时钟中断等内核事务后会返回到这里。

    # 恢复所有寄存器
    ld ra, 0(sp)
    ld gp, 16(sp)
    ld tp, 24(sp)
    ld t0, 32(sp)
    ld t1, 40(sp)
    ld t2, 48(sp)
    ld s0, 56(sp)
    ld s1, 64(sp)
    ld a0, 80(sp)
    ld a1, 88(sp)
    ld a2, 96(sp)
    ld a3, 104(sp)
    ld a4, 112(sp)
    ld a5, 120(sp)
    ld a6, 128(sp)
    ld a7, 136(sp)
    ld s2, 144(sp)
    ld s3, 152(sp)
    ld s4, 160(sp)
    ld s5, 168(sp)
    ld s6, 176(sp)
    ld s7, 184(sp)
    ld s8, 192(sp)
    ld s9, 200(sp)
    ld s10, 208(sp)
    ld s11, 216(sp)
    ld t3, 224(sp)
    ld t4, 232(sp)
    ld t5, 240(sp)
    ld t6, 248(sp)

    # 恢复栈指针（释放栈空间）
    addi sp, sp, 256

    # 从陷阱返回
    sret

hang:
    j hang

# =================================================================
#
# userret(trapframe_addr) - 从内核返回到用户空间
#
# 这个函数由 C 代码 usertrapret() 调用。
# a0: 存放了要恢复的陷阱帧的地址。
#
.global userret
userret:
    # -------------------------------------------------------------
    # 步骤 1: 恢复所有通用寄存器
    # -------------------------------------------------------------
    # C 代码 usertrapret 已经负责设置好了 satp, sepc, sstatus。
    # 我们只需要从 a0 指向的陷阱帧中恢复寄存器即可。
    ld ra, 0(a0)
    ld sp, 8(a0)      # 恢复 *用户* 的栈指针
    ld gp, 16(a0)
    ld tp, 24(a0)
    ld t0, 32(a0)
    ld t1, 40(a0)
    ld t2, 48(a0)
    ld s0, 56(a0)
    ld s1, 64(a0)
    ld a1, 88(a0)
    ld a2, 96(a0)
    ld a3, 104(a0)
    ld a4, 112(a0)
    ld a5, 120(a0)
    ld a6, 128(a0)
    ld a7, 136(a0)
    ld s2, 144(a0)
    ld s3, 152(a0)
    ld s4, 160(a0)
    ld s5, 168(a0)
    ld s6, 176(a0)
    ld s7, 184(a0)
    ld s8, 192(a0)
    ld s9, 200(a0)
    ld s10, 208(a0)
    ld s11, 216(a0)
    ld t3, 224(a0)
    ld t4, 232(a0)
    ld t5, 240(a0)
    ld t6, 248(a0)
    
    # 最后恢复 a0，因为之前我们一直用它作为基址寄存器。
    ld a0, 80(a0)
    
    # -------------------------------------------------------------
    # 步骤 2: 执行 sret 指令返回用户态
    # -------------------------------------------------------------
    # sret 指令会原子地执行以下操作:
    # 1. 将 PC 设置为 sepc 的值。
    # 2. 将权限级别设置为 sstatus.SPP 的值 (这里是 U-mode)。
    # 3. 将 sstatus.SIE 设置为 sstatus.SPIE 的值 (恢复中断使能状态)。
    sret
