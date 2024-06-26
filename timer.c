/*
 * 软件定时器模块，
 * 用于实现指定进程休眠的时长
 */
#include "defs.h"
#include "time.h"


#define TIMER_CODE  0x5AA5
static ListEntry_t kSleepList;


void init_timer (void)
{
    list_init (&kSleepList);
}

/* 添加软件定时器模块 */
struct Timer *timer_add (struct ProcCB *pcb, int expires)
{
    struct Timer *pTimer = NULL, *timer = NULL;
    ListEntry_t *plist = NULL;

    if ((pcb == NULL) || (expires == 0))
        kError(eSVC_Timer, E_PARAM);

    kDISABLE_INTERRUPT();
    timer = (struct Timer *)kalloc(sizeof(struct Timer));
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
            pTimer = list_container_of(plist, struct Timer, list);

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
void timer_del (struct Timer *timer)
{
    struct Timer *pTimer = NULL;

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
                            struct Timer, list);
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
    struct Timer *pTimer = NULL;
    struct ProcCB *pcb = NULL;

    pList = kSleepList.next;

    if (pList != &kSleepList)
    {
        pTimer = list_container_of(pList, struct Timer, list);

        if (pTimer->expires > 0)
            pTimer->expires -= 1;

        while(pTimer->expires == 0)
        {
            pList = pList->next;
            pcb = pTimer->pcb;

            proc_wakeup(pcb);
            if (pTimer->expires != 0)
            {
                if (pTimer->list.next != &kSleepList)
                {
                    pTimer = list_container_of(pTimer->list.next, 
                                struct Timer, list);
                    pTimer->expires += pTimer->expires;
                }
            }
            kDISABLE_INTERRUPT();
            list_del_init(&pTimer->list);
            kENABLE_INTERRUPT();

            if (pList == &kSleepList)
                break;

            pTimer = list_container_of(pList, struct Timer, list);
        }
    }
}
