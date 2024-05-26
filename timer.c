/*
 * 软件定时器模块，
 * 用于实现指定进程休眠的时长
 */
#include "defs.h"


#define TIMER_CODE  0x5AA5
static ListEntry_t kSleepList;


void init_timer (void)
{
    list_init (&kSleepList);
}

/* 添加软件定时器模块 */
timer_t *timer_add (ProcCB *pcb, int expires)
{
    timer_t *pTimer = NULL, *timer = NULL;
    ListEntry_t *plist = NULL;

    if ((pcb == NULL) || (expires == 0))
        kError(eSVC_Timer, E_PARAM);

    kDISABLE_INTERRUPT();
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
    kENABLE_INTERRUPT();
    return timer;
}

/* 删除软件定时器对象 */
void timer_del (timer_t *timer)
{
    timer_t *pTimer = NULL;

    if (timer == NULL)
        return;
    if (timer->code != TIMER_CODE)
        return;

    kDISABLE_INTERRUPT();
    if (!list_empty(&timer->list))
    {
        if (timer->expires != 0)
        {
            if (timer->list.next != &kSleepList)
            {
                pTimer = list_container_of(timer->list.next, 
                            timer_t, list);
                pTimer->expires += timer->expires;
            }
        }
        list_del_init(&timer->list);
    }
    kfree(timer);
    kENABLE_INTERRUPT();
}

/* 由定时器中断调用，实时处理软件定时器链表 */
void timer_run (void)
{
    ListEntry_t *pList = NULL;
    timer_t *pTimer = NULL;
    ProcCB *pcb = NULL;

    pList = kSleepList.next;

    if (pList != &kSleepList)
    {
        pTimer = list_container_of(pList, timer_t, list);

        if (pTimer->expires > 0)
            pTimer->expires -= 1;

        while(pTimer->expires == 0)
        {
            pList = pList->next;
            pcb = pTimer->pcb;

            wakeProcCB(pcb);
            if (pTimer->expires != 0)
            {
                if (pTimer->list.next != &kSleepList)
                {
                    pTimer = list_container_of(pTimer->list.next, 
                                timer_t, list);
                    pTimer->expires += pTimer->expires;
                }
            }
            kDISABLE_INTERRUPT();
            list_del_init(&pTimer->list);
            kENABLE_INTERRUPT();

            if (pList == &kSleepList)
                break;

            pTimer = list_container_of(pList, timer_t, list);
        }
    }
}
