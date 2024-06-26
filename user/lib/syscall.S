

.globl syscall
syscall:
    move t0,    a0
    move a0,    a1
    move a1,    a2
    move a2,    a3
    move a3,    a4
    move a4,    a5
    move a5,    a6
    move a6,    a7
    move a7,    t0
    ecall
    ret

