
#include <stdarg.h>
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"


#define console_putc    uartputc_sync
#define console_bufSize 128

typedef struct console_manage_info
{
    char buf[console_bufSize];
    ringbuf_t rb;
}consoleInfo_t;

static consoleInfo_t consState;



int console_wString (char *src)
{
    char *xStr = src;

    while (*xStr != '\0')
    {
        console_putc(*xStr);
        xStr++;
    }
    return 0;
}
int console_wCmd (char *src, int len)
{
    int i;
    char *xStr = src;

    for (i=0; i<len; i++, xStr++)
    {
        console_putc(*xStr);
    }
    return i;
}
void console_wChar (char src)
{
    console_putc(src);
}
int console_rCmd (char *src, int len)
{
    int idx = 0;
    char ch = 0;
    char *xStr = src;

    while (len > 0)
    {
        if (0 == kRingbuf_getState(&consState.rb))
            do_suspend(&consState.rb);
        kRingbuf_getChar(&consState.rb, &ch);

        xStr[idx++] = ch;
        len -= 1;
    }
    return len;
}
int console_rChar (void)
{
    int ch;

    if (0 == kRingbuf_getState(&consState.rb))
        do_suspend(&consState.rb);
    kRingbuf_getChar(&consState.rb, (char*)&ch);

    return ch;
}



void console_init (void)
{
    uart_init();
    kRingbuf_init(&consState.rb, consState.buf, console_bufSize);
    memset(&consState.buf, 0, console_bufSize);
}

void console_isr (int c)
{
    console_putc(c);
    kRingbuf_putChar(&consState.rb, (char)c);
    do_resume(&consState.rb);
}












