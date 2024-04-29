
#include "memlayout.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "list.h"


void process_init (void)
{
    console_wString("sh: ");

    while(1)
    {
        cli_main();
    }
}



void main (void)
{
    console_init();
    kmem_init();
    kvm_init();
    trap_init();
    trap_inithart();
    plic_init();
    plic_inithart();
    proc_init();
    cli_init();
    
    kprintf("Start OS ...\r\n");
    create(process_init);

    scheduler();
}

