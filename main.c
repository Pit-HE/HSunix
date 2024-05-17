
#include "memlayout.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "list.h"





void main (void)
{
    dev_init();
    console_init();
    kmem_init();
    // kvm_init();
    trap_init();
    plic_init();
    plic_inithart();
    cli_init();
    proc_init();
    timer_init();

    kprintf("Start OS ...\r\n");
    idle_main();
}

