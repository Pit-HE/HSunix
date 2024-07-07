/*
 * 用户模式下执行系统调用的汇编实现
 */

.globl syscall
syscall:
    # 将要传递的参数写入相关寄存器中
    move t0,    a0
    move a0,    a1
    move a1,    a2
    move a2,    a3
    move a3,    a4
    move a4,    a5
    move a5,    a6
    move a6,    a7
    move a7,    t0
    # 执行系统调用，切换到内核空间
    ecall
    ret

