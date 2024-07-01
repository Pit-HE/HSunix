/* 
 * 在系统移植时由外部实现的功能接口
 */

#include "defs.h"
#include "proc.h"


// /* 嵌套实现关闭中断 */
// void kPortDisableInterrupt (void)
// {
//     struct CpuCB *cpu = getCpuCB();

//     intr_off();

//     if (cpu->intrOffNest == 0)
//     {
//         cpu->intrOldState = intr_get();
//     }
//     cpu->intrOffNest++;
// }

// /* 嵌套实现开启中断 */
// void kPortEnableInterrupt (void)
// {
//     struct CpuCB *cpu = getCpuCB();

//     if (cpu->intrOffNest > 0)
//     {
//         if ((--cpu->intrOffNest) == 0)
//         {
//             if (cpu->intrOldState)
//                 intr_on();
//         }
//     }
// }

uint8 Os_interrupt = 0;
void k_enable_all_interrupt (void)
{
    Os_interrupt = 1;
    intr_on();
}

/* 嵌套实现关闭中断 */
void kPortDisableInterrupt (void)
{
    if (Os_interrupt == 0)
        return;
    intr_off();
}

/* 嵌套实现开启中断 */
void kPortEnableInterrupt (void)
{
    if (Os_interrupt == 0)
        return;
    intr_on();
}
