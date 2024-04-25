
#include "memlayout.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "list.h"


#define PROC_NUM_MAX    3
ProcCB_t    procUserCB[PROC_NUM_MAX];
char        procStack[PROC_NUM_MAX][4096];
extern uint Systicks;

void process_entry0 (void)
{
    static uint64 TmrCnt = 0;
    while(1)
    {
        if (++TmrCnt > 50)
        {
            TmrCnt = 0;
            kprintf ("%s\r\n", "Running");
        }
        sleep(&Systicks);
    }
}
void process_entry1 (void)
{
    static uint64 TmrCnt = 0;
    while(1)
    {
        if (++TmrCnt > 20)
        {
            TmrCnt = 0;
            kprintf ("%s\r\n", "Hello World");
        }
        sleep(&Systicks);
    }
}
void process_entry2 (void)
{
    static uint64 TmrCnt = 0;
    while(1)
    {
        if (++TmrCnt > 10)
        {
            TmrCnt = 0;
            kprintf ("%s\r\n", "HSunix");
        }
        sleep(&Systicks);
    }
}









void main (void)
{
    console_init();
    kinit();
    kvm_init();
    trap_init();
    trap_inithart();
    plic_init();
    plic_inithart();
    proc_init();

    kprintf("Start OS ...\r\n");

    create(&procUserCB[0], &procStack[0][2048], process_entry0);
    create(&procUserCB[1], &procStack[1][2048], process_entry1);
    create(&procUserCB[2], &procStack[2][2048], process_entry2);

    scheduler();
}

