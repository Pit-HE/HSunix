/* 
 * 在系统移植时由外部实现的功能接口
 */

#include "defs.h"

/* 嵌套实现关闭中断 */
void kPortDisableInterrupt (void)
{
    CpuCB *cpu = getCpuCB();

    intr_off();

    if (cpu->intrOffNest == 0)
    {
        cpu->intrOldState = intr_get();
    }
    cpu->intrOffNest++;
}

/* 嵌套实现开启中断 */
void kPortEnableInterrupt (void)
{
    CpuCB *cpu = getCpuCB();

    if (cpu->intrOffNest > 0)
    {
        if ((--cpu->intrOffNest) == 0)
        {
            if (cpu->intrOldState)
                intr_on();
        }
    }
}

