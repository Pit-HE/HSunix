
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "list.h"


void main (void)
{
    uartinit();
    console_init();

    char buf[] = "\nHello World !\n";
    char rbuf[128];
    int  rlen = 0;

    console_write(buf, strlen(buf));

    int i;

    while (1)
    {
        for (i=0; i<10000; i++)
        {
            console_write("sh: ", 5);
        
            memset(rbuf, 0, rlen);
            rlen = console_read(rbuf);
            
            if (rlen > 0)
                console_write(rbuf, rlen);
        }
    }
}


