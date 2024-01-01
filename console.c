
#include <stdarg.h>
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"


#define console_getc    uartgetc_loop
#define console_putc    uartputc_sync



int console_wCmd (char *src, int n)
{
    int i;
    char *xStr = src;

    for (i=0; i<n; i++, xStr++)
    {
        console_putc(*xStr);
    }
    return i;
}
void console_wChar (char *src)
{
    console_putc(*src);
}

int console_rCmd (char *src)
{
    int  len = 0;
    char chr = 0;
    char *xStr = src;

    while(1)
    {
        chr = console_getc();
        if (chr == '\r')
            chr = '\n';

        xStr[len++] = chr;
        console_putc(chr);

        if (chr == '\n')
            break;
    }
    return len;
}
int console_rChar (void)
{
    return console_getc();
}

void console_init (void)
{
}











