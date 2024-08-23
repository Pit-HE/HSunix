/*
 * risc-v 架构的 plic 功能管理模块
 */
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"


/* 初始化 riscv64 架构中的 plic 模块 */
void init_plic (void)
{
    /* 使能串口中断 */
    *(uint32*)(PLIC + UART0_IRQ*4) = 1;
    // 使能虚拟磁盘中断
    *(uint32*)(PLIC + VIRTIO0_IRQ*4) = 1; 
}

/* 初始化 plic 模块中管理的指定外设的中断功能 */
void init_plichart (void)
{
    int id = getCpuID();

    /* Enable uart interrupt */
    *(uint32*)(PLIC_SENABLE(id)) = (1 << UART0_IRQ) | (1 << VIRTIO0_IRQ);

    /* Open core interrupt priority zero */
    *(uint32*)(PLIC_SPRIORITY(id)) = 0;
}

/* 获取当前 cpu 发生的中断编号 */
int plic_claim (void)
{
    /* Get current interrupt number */
    return *(uint32*)PLIC_SCLAIM(getCpuID());
}

/* 清除当前 cpu 指定中断的标志位 */
void plic_complete (int irq)
{
    /* Clear current interrupt number */
    *(uint32*)PLIC_SCLAIM(getCpuID()) = irq;
}
