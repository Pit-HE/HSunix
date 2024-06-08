
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"


/****************************************/
uint64 Systicks;

/****************************************/
extern void kernelvec ();
extern void uservec (void);
extern void userret (void);
extern void trampoline (void);
extern void virtio_disk_isr(void);

/****************************************/
void init_trap(void)
{
    /* 设置特权模式下发生中断时的函数入口 */
    w_stvec((uint64)kernelvec);
}

/******************************************************
 *             外设中断与软件中断的处理函数
******************************************************/
int dev_interrupt (void)
{
    int irq, ret = 0;
    uint64 scause = r_scause();

    /***** 外部中断 *****/
    if ((scause & 0x8000000000000000L) &&
        (scause & 0xF) == 9)
    {
        irq = plic_claim();
        switch (irq)
        {
            case 0:
                break;
            case UART0_IRQ:
                uart_intrrupt();
                break;
            case VIRTIO0_IRQ:
                virtio_disk_isr();
                break;
            default:
                break;
        }
        if (irq)
            plic_complete(irq);
        ret = 1;
    }
    /***** 软件中断 *****/
    else if (scause == 0x8000000000000001L)
    {
        if (getCpuID() == 0)
        {
            Systicks++;
            timer_run();
        }

        w_sip(r_sip() & ~2);
        ret = 2;
    }
    else
    {
        kError(eSVC_Interrupt, E_STATUS);
    }

    return ret;
}

/******************************************************
 *             用户模式的 trap 处理代码
******************************************************/
/* 处理用户模式下发生的 trap 事件 */
void trap_userfunc(void)
{
    uint devnum = 0;
    ProcCB *pcb = getProcCB();

    /* 判断 trap 是否来自用户模式 */
    if ((r_sstatus() & SSTATUS_SPP) != 0)
      kError(eSVC_Trap, E_STATUS);

    /* 避免发生中断嵌套,
     * 在特权模式下的中断应由内核处理
     */
    w_stvec((uint64)kernelvec);
    pcb->trapFrame->epc = r_sepc();

    /***** 系统调用 *****/
    if (r_scause() == 8)
    {
        if (KillState(pcb))
          do_exit(-1);

        pcb->trapFrame->epc += 4;
        intr_on();
        do_syscall();
    }
    /***** 外部中断、软件中断、非法故障 *****/
    else
    {
        devnum = dev_interrupt();
        /* 发生非法故障，杀死当前进程 */
        if (devnum == 0)
            do_kill(pcb->pid);
    }

    /* 中断发生后的处理 */
    if (KillState(pcb))
        do_exit(-1);
    if (devnum == 2)
        do_yield();

    /* 返回用户空间 */
    trap_userret();
}

/* 处理从 trap 中返回用户模式 */
void trap_userret(void)
{
    unsigned long x;
    ProcCB *pcb = getProcCB();

    intr_off();

    /* 设置用户模式 trap 入口函数 */
    w_stvec((uint64)(TRAMPOLINE + (uservec - trampoline)));

    /* 页表寄存器 */
    pcb->trapFrame->kernel_satp = r_satp();
    /* 重置进程栈地址 */
    pcb->trapFrame->kernel_sp = pcb->stackAddr + PGSIZE;
    /* 设置用户进程中断入口(给 uservec 读取) */
    pcb->trapFrame->kernel_trap = (uint64)trap_userfunc;
    /* 记录当前的 cpu ID */
    pcb->trapFrame->kernel_hartid = r_tp();

    /* 设置用户模式的信息 */
    x = r_sstatus();
    x &= ~SSTATUS_SPP;  // 设置为用户模式
    x |=  SSTATUS_SPIE; // 开启用户模式的中断
    w_sstatus(x);

    /* 恢复 trap_userfunc 中保存的 epc 寄存器 */
    w_sepc(pcb->trapFrame->epc);

    /* 获取当前进程用户空间的页表地址 */
    uint64 satp = MAKE_SATP(pcb->pageTab);

    /* 跳转到汇编的用户空间返回代码 */
    uint64 trampoline_userret = TRAMPOLINE + (userret - trampoline);
    ((void (*)(uint64))trampoline_userret)(satp);
}

/******************************************************
 *             监督者模式的 trap 处理代码
******************************************************/
/* 监督者模式中断发生时的处理函数 */
void kerneltrap(void)
{
    int devnum = 0;
    ProcCB *pcb;
    uint64 sepc = r_sepc();
    uint64 sstatus = r_sstatus();
    // uint64 scause = r_scause();

    /* 有效性检查 */
    if ((sstatus & SSTATUS_SPP)==0)
      kError(eSVC_Trap, E_STATUS);
    if (intr_get() != 0)
      kError(eSVC_Trap, E_INTERRUPT);

    /* 处理外设中断与软件中断 */
    devnum = dev_interrupt();
    if (devnum == 0)
    {
        kError(eSVC_Trap, E_INTERRUPT);
    }
    pcb = getProcCB();

    /* 是否为定时器中断 */
    if ((devnum == 2) && (pcb != NULL) &&
        (pcb->state == RUNNING))
        do_yield();

    // 恢复存储的寄存器
    w_sepc(sepc);
    w_sstatus(sstatus);
}
