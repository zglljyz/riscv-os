# 系统调用包装 - 汇编实现
# 每个系统调用函数将系统调用号放入a7，然后执行ecall

.global fork
fork:
    li a7, 1        # SYS_fork
    ecall
    ret

.global exit
exit:
    li a7, 2        # SYS_exit
    ecall
    ret

.global wait
wait:
    li a7, 3        # SYS_wait
    ecall
    ret

.global getpid
getpid:
    li a7, 4        # SYS_getpid
    ecall
    ret

.global kill
kill:
    li a7, 5        # SYS_kill
    ecall
    ret

.global read
read:
    li a7, 6        # SYS_read
    ecall
    ret

.global write
write:
    li a7, 7        # SYS_write
    ecall
    ret

.global open
open:
    li a7, 8        # SYS_open
    ecall
    ret

.global close
close:
    li a7, 9        # SYS_close
    ecall
    ret

.global setpriority
setpriority:
    li a7, 8       # SYS_setpriority
    ecall
    ret

.global getpriority
getpriority:
    li a7, 9       # SYS_getpriority
    ecall
    ret
