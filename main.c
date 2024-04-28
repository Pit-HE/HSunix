
#include "memlayout.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "list.h"


extern uint Systicks;

void process_entry0 (void)
{
    static uint64 TmrCnt0 = 0;
    while(1)
    {
        if (++TmrCnt0 > 50)
        {
            TmrCnt0 = 0;
            kprintf ("%s\r\n", "Running");
        }
        sleep(&Systicks);
    }
}
void process_entry1 (void)
{
    static uint64 TmrCnt1 = 0;
    while(1)
    {
        if (++TmrCnt1 > 20)
        {
            TmrCnt1 = 0;
            kprintf ("%s\r\n", "Hello World");
        }
        sleep(&Systicks);
    }
}
void process_entry2 (void)
{
    static uint64 TmrCnt2 = 0;
    while(1)
    {
        if (++TmrCnt2 > 10)
        {
            TmrCnt2 = 0;
            kprintf ("%s\r\n", "HSunix");
        }
        sleep(&Systicks);
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
    ksmall_init();

    kprintf("Start OS ...\r\n");

    create(process_entry2);
    create(process_entry1);
    create(process_entry0);

    scheduler();
}

