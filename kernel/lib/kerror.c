/*
 * 内核错误处理模块，打印错误信息
 */
#include "defs.h"
#include "proc.h"
#include "pcb.h"


void ErrPrint (char *fmt, ...)
{
    va_list va;
    struct ProcCB *pcb = getProcCB();

    kprintf ("Fail: ");

    /* 打印要处理的不定长参数 */
    va_start(va, fmt);
    tfp_format(NULL, console_wChar, fmt, va);
    va_end(va);

    /* 打印故障进程的信息 */
    kprintf ("Current PCB = %s, PID = %d\r\n", 
            pcb->name, pcb->pid);
    kprintf ("----------------------------\r\n");

    while(1)
    {}
}
