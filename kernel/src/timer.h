
#ifndef __TIMER_H__
#define __TIEMR_H___


#include "types.h"
#include "list.h"
#include "proc.h"


struct Timer
{
    /* 标记 */
    int             magic;
    /* 软件定时器的超时计数 */
    uint64          expires;
    /* 链表节点 */
    ListEntry_t     list;
    /* 软件定时器管理的进程控制块 */
    struct ProcCB  *pcb;
};



#endif

