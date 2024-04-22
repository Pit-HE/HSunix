
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

void plic_init (void)
{
    /* Set uart interrupt priority */
    *(uint32*)(PLIC + 4*UART0_IRQ) = 1;
}

void plic_inithart (void)
{
    /* Enable uart interrupt */
    *(uint32*)(PLIC_SENABLE(cpuid())) = (1 << UART0_IRQ);

    /* Open core interrupt priority zero */
    *(uint32*)(PLIC_SPRIORITY(cpuid())) = 0;
}

int plic_claim (void)
{
    /* Get current interrupt number */
    return *(uint32*)PLIC_SCLAIM(cpuid());
}

void plic_complete (int irq)
{
    /* Clear current interrupt number */
    *(uint32*)PLIC_SCLAIM(cpuid()) = irq;
}
