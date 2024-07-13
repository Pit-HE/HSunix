/*
 * 内核错误处理模块，打印错误信息
 */
#include "defs.h"


void ErrPrint (char *fmt, ...)
{
    va_list va;
    struct ProcCB *pcb = getProcCB();

    kprintf ("Fail: ");

    va_start(va, fmt);
    tfp_format(NULL, console_wChar, fmt, va);
    va_end(va);

    kprintf ("Current PCB = %s, PID = %d\r\n", 
            pcb->name, pcb->pid);
    kprintf ("----------------------------\r\n");

    while(1)
    {}
}
