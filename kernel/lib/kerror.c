
#include "defs.h"
#include "kerror.h"

void kError (eService SVC, eCode code)
{
    kprintf ("error: SCV = %d, code = %d", SVC, code);

    while(1)
    {}
}

void kErrPrintf (char *fmt, ...)
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
