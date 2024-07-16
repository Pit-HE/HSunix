/*
 * 用户层的基础功能的线程库
 */
#include "libc.h"
#include "pthread.h"


/* 实现线程切换的关键函数 */
extern void thread_switch (struct Context *old, struct Context *new);

/* 当前正在运行的线程 */
static ThreadCB   *currTCB;
/* 空闲线程的控制块 */
static ThreadCB    idleTCB;
/* 线程 ID 令牌 */
static pthread_t   tid_token;
/* 线程就绪链表 */
static ListEntry_t pt_readlist;
/* 线程休眠链表 */
static ListEntry_t pt_sleeplist;
/* 线程退出链表 */
static ListEntry_t pt_exitlist;


/* 线程调度器 */
static ThreadCB *pthread_scheduler (void)
{
    ThreadCB *tcb = NULL;
    ListEntry_t *ptr = NULL;
    ListEntry_t *qtr = NULL;

    while (1)
    {
        /* 处理有线程退出的情况 */
        if (FALSE == list_empty(&pt_exitlist))
        {
            tcb = list_container_of(pt_exitlist.next,
                    ThreadCB, list);
            break;
        }

        /* 管理休眠的线程 */
        list_for_each_safe (ptr, qtr, &pt_sleeplist)
        {
            tcb = list_container_of(ptr, ThreadCB, list);
            if (tcb->sleep > 1)
            {
                tcb->sleep -= 1;
                continue;
            }

            list_del(&tcb->list);
            tcb->stat = READY;
            tcb->sleep = 0;
            list_add(&pt_readlist, &tcb->list);
        }

        /* 管理就绪的线程 */
        list_for_each_safe (ptr, qtr, &pt_sleeplist)
        {
            currTCB = list_container_of(ptr, ThreadCB, list);
            currTCB->stat = RUNNING;

            /* 切换到新的进程 */
            thread_switch(&idleTCB.context, &currTCB->context);
            currTCB = &idleTCB;
        }

        /* 非标准做法：用休眠来提供线程的时基
         * ( 标准做法应该是内核提供定时器功能 )
         */
        sleep(1);
    }

    return tcb;
}
/* 所有线程的入口 */
static void pthread_entry (ThreadCB *tcb)
{
    void *ret = NULL;

    if ((tcb == NULL) || (tcb->entry == NULL))
        return;

    ret = tcb->entry(tcb->arg);
    pthread_exit(ret);
}


/* 初始化整个线程管理模块 */ 
void init_pthread (void)
{
    tid_token = 1;

    /* 初始化线程模块内部的管理链表 */
    list_init(&pt_readlist);
    list_init(&pt_sleeplist);
    list_init(&pt_exitlist);

    /* 初始化空闲线程的控制块 */
    memset(&idleTCB, 0, sizeof(ThreadCB));
    list_init(&idleTCB.list);
    idleTCB.tid = 0;

    /* 设置主线程 */
    currTCB = &idleTCB;
}


/* 创建线程 
 * ( 目前不接受线程属性的设置功能 )
 */
int pthread_create (pthread_t *thread, const pthread_attr_t *attr, 
        void *(*start_routine)(void *), void *arg)
{
    void *stack = NULL;
    ThreadCB *tcb = NULL;
    pthread_t tid = tid_token++;

    if ((thread == NULL) || (start_routine == NULL))
        return -1;
    
    stack = malloc(1024);
    if (stack == NULL)
        return -1;
    tcb = malloc(sizeof(ThreadCB));
    if (tcb == NULL)
    {
        free(stack);
        return -1;
    }

    *thread = tid;

    /* 初始化线程控制块 */
    tcb->entry      = start_routine;
    tcb->context.ra = (uint64)pthread_entry;
    tcb->context.sp = (uint64)stack;
    tcb->tid        = tid;
    tcb->stat       = IDLE;
    tcb->exitval    = NULL;
    tcb->attr       = attr;
    tcb->arg        = arg;
    tcb->stack      = stack;

    list_init(&tcb->list);
    list_add (&pt_readlist, &tcb->list);

    /* 设置第一个执行的线程 */
    if (currTCB == NULL)
        currTCB = tcb;

    return 0;
}

/* 退出线程 */
int pthread_exit (void *retval)
{
    ThreadCB *tcb = currTCB;

    if (tcb == NULL)
        return -1;

    /* 移动线程所属的链表 */
    list_del(&tcb->list);
    list_add(&pt_exitlist, &tcb->list);

    tcb->stat = EXITING;
    tcb->exitval = retval;    

    return 0;
}

/* 等待指定线程退出 */
int pthread_join(pthread_t thread, void **retval)
{
    pthread_t tid;
    ThreadCB *tcb = NULL;

    do
    {
        /* 执行调度器功能 */
        tcb = pthread_scheduler();
        tid = tcb->tid;

        /* 释放退出线程所占用的资源 */
        if (tid == thread)
            *retval = tcb->exitval;
        list_del(&tcb->list);
        free(tcb->stack);
        free(tcb);
    }while(tid != thread);
    
    return tid;
}

/* 线程休眠 */
int pthread_sleep(int ms)
{
    ThreadCB *tcb = currTCB;

    if (tcb == NULL)
        return -1;

    /* 移动线程所属的链表 */
    list_del(&tcb->list);
    list_add(&pt_sleeplist, &tcb->list);

    tcb->stat = SLEEPING;
    tcb->sleep = ms;

    return 0;
}

