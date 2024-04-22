
#include "memlayout.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "list.h"

extern void tc_init (void);
extern void tc_main (void);

void _error (void)
{
    while(1)
    {
        ;
    }
}

void main (void)
{
    static uint flip;
    console_init();
    // kinit();
    // kvm_init();
    // kvm_enable();
    trap_init();
    trap_inithart();
    plic_init();
    plic_inithart();

    // tc_init();

    console_wString("sh: \r\n");

    intr_on();
    while (1)
    {
        extern uint ticks;
        if (ticks >= (flip + 10))
        {
            flip = ticks;
            // console_wString("sh: \r\n");
        }
        
        // tc_main();
    }
}

uint32 printcnt = 0;
void printtest (void)
{
    printcnt ++;
}
