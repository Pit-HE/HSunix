
#ifndef __TIMER_H__
#define __TIEMR_H___


#include "types.h"
#include "list.h"
#include "proc.h"


struct Timer
{
    int             code;
    uint64          expires;
    ListEntry_t     list;
    struct ProcCB  *pcb;
};



#endif

