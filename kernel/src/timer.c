/*
 * 软件定时器模块，
 * ( 目前用于实现进程休眠指定时长 )
 */
#include "defs.h"
#include "time.h"


#define TIMER_MAGIC  0x5AA5
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
        ErrPrint("timer_add: Illegal function argument !\r\n");

    timer = (struct Timer *)kalloc(sizeof(struct Timer));
    if (timer != NULL)
    {
        timer->magic = TIMER_MAGIC;
        timer->expires = expires;
        list_init(&timer->list);
        timer->pcb = pcb;

        /* 将内核线程从进程管理模块移除 */
        kDISABLE_INTERRUPT();
        pcb->state = SLEEPING;
        list_del_init(&pcb->list);
        kENABLE_INTERRUPT();

        /* 遍历定时器的管理链表，查找当前定时器对象加入链表的位置 */
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
        /* 将定时器对象添加到管理链表中 */
        kDISABLE_INTERRUPT();
        list_add_before(plist, &timer->list);
        kENABLE_INTERRUPT();
    }
    
    return timer;
}

/* 删除软件定时器对象 */
void timer_del (struct Timer *timer)
{
    struct Timer *pTimer = NULL;

    if (timer == NULL)
        return;
    if (timer->magic != TIMER_MAGIC)
        return;

    if (!list_empty(&timer->list))
    {
        if (timer->expires != 0)
        {
            /* 更新当前软件定时器所在链表中的下一个定时器对象的计数值 */
            if (timer->list.next != &kSleepList)
            {
                pTimer = list_container_of(timer->list.next, 
                            struct Timer, list);
                pTimer->expires += timer->expires;
            }
        }
        /* 将定时器从管理链表中移除 */
        kDISABLE_INTERRUPT();
        list_del_init(&timer->list);
        kENABLE_INTERRUPT();
    }
    kfree(timer);
}

/* 由定时器中断调用，实时处理软件定时器的管理链表 */
void timer_run (void)
{
    ListEntry_t *pList = NULL;
    struct Timer *pTimer = NULL;
    struct ProcCB *pcb = NULL;

    pList = kSleepList.next;

    if (pList != &kSleepList)
    {
        /* 获取软件定时器管理链表中的第一个定时器对象 */
        pTimer = list_container_of(pList, struct Timer, list);
        if (pTimer->expires > 0)
            pTimer->expires -= 1;

        /* 处理多个软件定时器同时到期的情况 */
        while(pTimer->expires == 0)
        {
            pList = pList->next;
            pcb = pTimer->pcb;

            proc_wakeup(pcb);
            if (pTimer->expires != 0)
            {
                /* 更新当前软件定时器所在链表中的下一个定时器对象的计数值 */
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

            /* 是否已经完成管理链表的遍历 */
            if (pList == &kSleepList)
                break;

            /* 切换到下一个链表节点 */ 
            pTimer = list_container_of(pList, struct Timer, list);
        }
    }
}
