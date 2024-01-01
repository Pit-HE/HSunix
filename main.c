
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "list.h"

extern void tc_init (void);
extern void tc_main (void);

void error (void)
{
    while(1)
    {
        ;
    }
}

void main (void)
{
    uartinit();
    console_init();
    kinit();
    kvm_init();
    kvm_enable();

    tc_init();

    // char *pa;
    while (1)
    {
        console_wCmd("sh: ", 5);

        // pa = kalloc();
        // rlen = console_rCmd(rbuf);

        // if (rlen > 0)
        // {
        //     console_wCmd(rbuf, rlen);
        // }
        // console_wChar("\n");
        // kfree(pa);

        // tc_main();
    }
}


