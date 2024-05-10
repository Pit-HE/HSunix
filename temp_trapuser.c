
/*
 *
 * 当前文件需要配合 temp_retuser.S 一起使用，
 * 两个 temp_* 文件是用于测试模式切换是否顺利的，
 * 测试用户模式与特权模式彼此切换是否能正常工作
 *
 */
#include "defs.h"

/*******************************/
/* S file function extern */
void kernelvec ();
void uservec (void);
void userret (void);
void trampoline (void);
void temp_uservec (int *tramp);
void temp_userret (int *tramp);

int dev_interrupt (void);
/*******************************/
void user_trap (void);
void user_ret (void);


/* 进程在用户空间(模式)中执行的函数 */ 
uint64 userCnt = 0;
void user_processEntry (void)
{
    while(1)
    {
        userCnt++;

        void syscall_sleep (int ms);
        syscall_sleep(1000);
    }
}

/* 处理进程从用户模式切换到特权模式的功能 */
void user_trap (void)
{
    int flag;
    ProcCB_t *pcb;

    pcb = getProcCB();
    /* 避免进程在特权模式下又重复发生特权模式的中断 */ 
    w_stvec((uint64)kernelvec);

    /* 保存进程进入特权模式前在用户空间执行的代码地址 */
    pcb->trapFrame->epc = r_sepc();

    if (r_scause() == 8)
    {
        pcb->trapFrame->epc += 4;

        intr_on();
        do_syscall();
    }
    else
    {
        /* 总的中断服务函数,处理此次发生的中断事件 */
        flag = dev_interrupt();
    }

    if (flag == 2)
        do_yield();

    user_ret();
}

/* 设置进程从特权模式切换到用户模式前的信息 */
void user_ret (void)
{
    ProcCB_t *pcb = getProcCB();

    intr_off();
    /* 恢复用户模式下发生异常的入口 */
    w_stvec((uint64)temp_uservec);

    /* 记录当前特权模式下的内核信息，
     * 用于切换进入特权模式前恢复内核的工作状态
     */
    pcb->trapFrame->kernel_satp = r_satp();
    /* 内核工作的栈地址(这是最核心的一步) */
    pcb->trapFrame->kernel_sp = (uint64)r_sp();
    // pcb->trapFrame->kernel_sp = (uint64)pcb->stackAddr + pcb->stackSize;
    pcb->trapFrame->kernel_trap = (uint64)user_trap;
    pcb->trapFrame->kernel_hartid = r_tp();

    unsigned long x = r_sstatus();
    x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
    x |= SSTATUS_SPIE; // enable interrupts in user mode
    w_sstatus(x);

    /* 设置sret后，要执行的用户空间代码的地址 */
    w_sepc(pcb->trapFrame->epc);

    temp_userret((int *)pcb->trapFrame);
}
