
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


/******************************************/
/* 控制台原始接口 */
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

/******************************************/
/* 控制台为注册成内核文件设备而封装的接口 */
static int console_dev_write (struct Inode *inode,
    void *buf, unsigned int count)
{
    return console_wCmd(buf, count);
}
static int console_dev_read (struct Inode *inode,
    void *buf, unsigned int count)
{
    return console_rCmd(buf, count);
}



/* 控制台的中断服务函数 */
void console_isr (int c)
{
    console_putc(c);
    kRingbuf_putChar(&consState.rb, (char)c);
    do_resume(&consState.rb);
}
void init_console (void)
{
    struct Device *dev;

    uart_init();
    kRingbuf_init(&consState.rb, consState.buf, console_bufSize);
    kmemset(&consState.buf, 0, console_bufSize);

    /* 向内核注册 console 设备 */
    dev = dev_alloc("console");
    if (dev != NULL)
    {
        dev->opt.write = console_dev_write;
        dev->opt.read  = console_dev_read;

        dev_register(dev);
    }
}














