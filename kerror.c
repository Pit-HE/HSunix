
#include "defs.h"
#include "kerror.h"

void kError (eService SVC, eCode code)
{
    kprintf ("error: SCV = %d, code = %d", SVC, code);

    while(1)
    {}
}
