
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

void init_plic (void)
{
    /* Set uart interrupt priority */
    *(uint32*)(PLIC + 4*UART0_IRQ) = 1;
}

void init_plichart (void)
{
    /* Enable uart interrupt */
    *(uint32*)(PLIC_SENABLE(getCpuID())) = (1 << UART0_IRQ);

    /* Open core interrupt priority zero */
    *(uint32*)(PLIC_SPRIORITY(getCpuID())) = 0;
}

int plic_claim (void)
{
    /* Get current interrupt number */
    return *(uint32*)PLIC_SCLAIM(getCpuID());
}

void plic_complete (int irq)
{
    /* Clear current interrupt number */
    *(uint32*)PLIC_SCLAIM(getCpuID()) = irq;
}
