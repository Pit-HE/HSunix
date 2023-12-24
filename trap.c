
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"



void usertrap(void)
{

}

void usertrapret(void)
{

}

void kerneltrap(void)
{
    uint64 sepc = r_sepc();
    uint64 sstatus = r_sstatus();

    // the yield() may have caused some traps to occur,
    // so restore trap registers for use by kernelvec.S's sepc instruction.
    w_sepc(sepc);
    w_sstatus(sstatus);
}

