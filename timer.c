
#include "defs.h"


#define TIMER_CODE  0x5AA5
static list_entry_t kSleepList;


void timer_init (void)
{
    list_init (&kSleepList);
}

timer_t *timer_add (ProcCB_t *pcb, int expires)
{
    timer_t *pTimer, *timer;
    list_entry_t *plist;

    if ((pcb == NULL) || (expires == 0))
        kError(eSVC_Timer, E_PARAM);

    timer = (timer_t *)kalloc(sizeof(timer_t));
    if (timer != NULL)
    {
        timer->code = TIMER_CODE;
        timer->expires = expires;
        list_init(&timer->list);
        timer->pcb = pcb;

        pcb->state = SLEEPING;
        list_del_init(&pcb->list);

        list_for_each(plist, &kSleepList)
        {
            pTimer = list_container_of(plist, timer_t, list);

            if (timer->expires < pTimer->expires)
            {
                pTimer->expires -= timer->expires;
                break;
            }
            timer->expires -= pTimer->expires;
        }
        list_add_before(plist, &timer->list);
    }
    return timer;
}

void timer_del (timer_t *timer)
{
    timer_t *pTimer;

    if (timer == NULL)
        return;
    if (timer->code != TIMER_CODE)
        return;

    if (!list_empty(&timer->list))
    {
        if (timer->expires != 0)
        {
            if (timer->list.next != &kSleepList)
            {
                pTimer = list_container_of(timer->list.next, timer_t, list);
                pTimer->expires += timer->expires;
            }
        }
        list_del_init(&timer->list);
    }
    kfree(timer);
}

void timer_run (void)
{
    list_entry_t *pList;
    timer_t *pTimer;
    ProcCB_t *pcb;

    pList = kSleepList.next;

    if (pList != &kSleepList)
    {
        pTimer = list_container_of(pList, timer_t, list);
        pTimer->expires -= 1;

        while(pTimer->expires == 0)
        {
            pList = pList->next;
            pcb = pTimer->pcb;

            wakeProcCB(pcb);
            timer_del(pTimer);

            if (pList == &kSleepList)
                break;

            pTimer = list_container_of(pList, timer_t, list);
        }
    }
}
