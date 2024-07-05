#
# 模拟 trampoline.S 文件的写法，
# 实现了没有 MMU 的模式切换功能，
# 验证了用户模式与特权模式互相切换的功能
#


#include "riscv.h"
#include "memlayout.h"


# void temp_uservec (int *tramp)
.globl temp_uservec
temp_uservec:
    csrr t0, sscratch
    sd a0,   112(t0)

    # 内存屏障
    sfence.vma zero, zero

    # 读取特权模式进入用户模式前，
    # 进程控制块中的 pcb->trapFrame 信息
    csrr a0, sscratch

    # 内存屏障
    sfence.vma zero, zero

    sd ra,   40(a0)
    sd sp,   48(a0)
    sd gp,   56(a0)
    sd tp,   64(a0)
    sd t0,   72(a0)
    sd t1,   80(a0)
    sd t2,   88(a0)
    sd s0,   96(a0)
    sd s1,  104(a0)
    sd a1,  120(a0)
    sd a2,  128(a0)
    sd a3,  136(a0)
    sd a4,  144(a0)
    sd a5,  152(a0)
    sd a6,  160(a0)
    sd a7,  168(a0)
    sd s2,  176(a0)
    sd s3,  184(a0)
    sd s4,  192(a0)
    sd s5,  200(a0)
    sd s6,  208(a0)
    sd s7,  216(a0)
    sd s8,  224(a0)
    sd s9,  232(a0)
    sd s10, 240(a0)
    sd s11, 248(a0)
    sd t3,  256(a0)
    sd t4,  264(a0)
    sd t5,  272(a0)
    sd t6,  280(a0)

    # 恢复操作系统内核之前的工作信息
    ld tp,  32(a0)  // kernel_tp
    ld t0,  24(a0)  // kernel_trap
    ld sp,  16(a0)  // kernel_sp
    ld t1,   0(a0)  // kernel_satp

    jr t0


# void temp_userret (int *tramp)
.globl temp_userret
temp_userret:
    # 保存进程控制块的 pcb->trapFrame 信息
    csrw sscratch, a0

    # 内存屏障
    sfence.vma zero, zero

    ld ra,   40(a0)
    ld sp,   48(a0)
    ld gp,   56(a0)
    ld tp,   64(a0)
    ld t0,   72(a0)
    ld t1,   80(a0)
    ld t2,   88(a0)
    ld s0,   96(a0)
    ld s1,  104(a0)
    ld a1,  120(a0)
    ld a2,  128(a0)
    ld a3,  136(a0)
    ld a4,  144(a0)
    ld a5,  152(a0)
    ld a6,  160(a0)
    ld a7,  168(a0)
    ld s2,  176(a0)
    ld s3,  184(a0)
    ld s4,  192(a0)
    ld s5,  200(a0)
    ld s6,  208(a0)
    ld s7,  216(a0)
    ld s8,  224(a0)
    ld s9,  232(a0)
    ld s10, 240(a0)
    ld s11, 248(a0)
    ld t3,  256(a0)
    ld t4,  264(a0)
    ld t5,  272(a0)
    ld t6,  280(a0)

    # 恢复 a0 寄存器的值
    ld a0, 112(a0)

    sret
