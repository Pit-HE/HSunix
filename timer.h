
#ifndef __TIMER_H__
#define __TIEMR_H___


#include "defs.h"


typedef struct kernelTimer
{
    int             code;
    uint64          expires;
    list_entry_t    list;
    ProcCB_t        *pcb;
}timer_t;


#endif

