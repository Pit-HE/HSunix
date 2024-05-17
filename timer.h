
#ifndef __TIMER_H__
#define __TIEMR_H___


#include "defs.h"


typedef struct kernelTimer
{
    int             code;
    uint64          expires;
    ListEntry_t    list;
    ProcCB        *pcb;
}timer_t;


#endif

