
#include <stdarg.h>
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"


static void console_putc (int c)
{
    uartputc_sync(c);
}

int console_write (char *src, int n)
{
    int i;
    char *xStr = src;

    for (i=0; i<n; i++, xStr++)
    {
        console_putc(*xStr);
    }
    return i;
} 

int console_read (char *src)
{
    int  len = 0;
    char chr = 0;
    char *xStr = src;

    while(1)
    {
        chr = uartgetc_loop();
        if (chr == '\r')
            chr = '\n';

        xStr[len++] = chr;
        console_putc(chr);

        if (chr == '\n')
            break;
    }
    return len;
}

void console_init (void)
{

}











