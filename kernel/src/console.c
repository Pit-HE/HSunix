/*
 * 控制台模块，提供控制台对字符读写的接口
 */
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

/* 往控制台输入字符串 */
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

/* 往控制台输入单个字符 */
void console_wChar (void *v, char src)
{
    console_putc(src);
}

/* 从控制台读取指定长度的字符串 */
int console_rCmd (char *src, int len)
{
    int idx = 0;
    char ch = 0;
    char *xStr = src;

    while (len > 0)
    {
        /* 从循环队列中获取串口数据，
         * 当队列为空时，挂起当前进程等待新的数据
         */
        if (0 == kRingbuf_getState(&consState.rb))
            do_suspend(&consState.rb);
        kRingbuf_getChar(&consState.rb, &ch);

        xStr[idx++] = ch;
        len -= 1;
    }
    return len;
}

/* 从控制台读取单个字符 */
int console_rChar (void)
{
    int ch;

    /* 从控制台专属的循环队列中读取单个字节 */
    if (0 == kRingbuf_getState(&consState.rb))
        do_suspend(&consState.rb);
    kRingbuf_getChar(&consState.rb, (char*)&ch);

    return ch;
}


/******************************************/
/* 控制台为注册成内核文件设备而封装的接口 */
static int console_dev_write (struct File *file,
    void *buf, uint count)
{
    return console_wCmd(buf, count);
}
static int console_dev_read (struct File *file,
    void *buf, uint count)
{
    return console_rCmd(buf, count);
}
/* 将控制台注册到内核的虚拟文件系统
 * ( 贯彻 unix 一切皆文件的思想 )
 */
static struct FileOperation console_dev_opt = 
{
    .write = console_dev_write,
    .read  = console_dev_read,
};


/* 控制台的中断服务函数 */
void console_isr (int c)
{
    /* 将数据写入控制台专属的循环队列，
     * 并唤醒等待在当前队列上的进程
     */
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
        dev->opt = &console_dev_opt;
        dev_register(dev);
    }

    init_printf(NULL, console_wChar);
}














