
#include "types.h"
#include "riscv.h"

int test = 0, out = 10, in = 99;

void main (void)
{
    test = 110;
    out  = 120;
    in   = 119;

    int i, j;

    while (1)
    {
        for (i=0; i<10000; i++)
        {
            for (j=0; j<999; j++)
            {
                
            }
        }
    }
}


void kerneltrap (void)
{
    uint64 sepc = r_sepc();
    uint64 sstatus = r_sstatus();

    // the yield() may have caused some traps to occur,
    // so restore trap registers for use by kernelvec.S's sepc instruction.
    w_sepc(sepc);
    w_sstatus(sstatus);
}
