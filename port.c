

#include "defs.h"

void kPortDisableInterrupt (void)
{
    CpuCB_t *cpu = getCpuCB();

    intr_off();

    if (cpu->intrOffNest == 0)
    {
        cpu->intrOldState = intr_get();
    }
    cpu->intrOffNest++;
}

void kPortEnableInterrupt (void)
{
    CpuCB_t *cpu = getCpuCB();

    if (cpu->intrOffNest > 0)
    {
        if ((--cpu->intrOffNest) == 0)
        {
            if (cpu->intrOldState)
                intr_on();
        }
    }
}

