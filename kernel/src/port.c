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

static uint8 Os_interrupt = 0;


/* 开启当前芯片的中断功能 */
void k_enable_all_interrupt (void)
{
    Os_interrupt = 1;
    intr_on();
}

/* 嵌套实现关闭中断 */
void kPortDisableInterrupt (void)
{
    struct CpuCB *cpu = getCpuCB();

    /* 判断是否开启中断管理功 */
    if (Os_interrupt == 0)
        return;
    
    /* 关中断可以重复执行，不必担心嵌套 */
    intr_off();
    cpu->intrOffNest++;
}

/* 嵌套实现开启中断 */
void kPortEnableInterrupt (void)
{
    struct CpuCB *cpu = getCpuCB();

    /* 判断是否开启中断管理功 */
    if (Os_interrupt == 0)
        return;

    /* 开启中断时需要判断嵌套的情况 */
    if (cpu->intrOffNest > 0)
    {
        if ((--cpu->intrOffNest) == 0)
            intr_on();
    }
}
