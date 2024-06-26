
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
    va_start(va, fmt);
    tfp_format(NULL, console_wChar, fmt, va);
    va_end(va);

    while(1)
    {}
}
