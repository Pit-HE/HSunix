/*
 * 用户层的基础功能的线程库
 */
#include "libc.h"
#include "list.h"
#include "pthread.h"


/* 实现线程切换的关键函数 */
extern void thread_switch (struct Context *old, struct Context *new);

/* 当前正在运行的线程 */
static ThreadCB   *currTCB;
/* 空闲线程的控制块 */
static ThreadCB   *idleTCB;
/* 线程 ID 令牌 */
static pthread_t   tid_token;
/* 线程就绪链表 */
static ListEntry_t pt_readylist;
/* 线程休眠链表 */
static ListEntry_t pt_sleeplist;
/* 线程退出链表 */
static ListEntry_t pt_exitlist;


/* 线程调度器 */
static ThreadCB *pthread_scheduler (pthread_t tid)
{
    ThreadCB *tcb = NULL;
    ListEntry_t *ptr = NULL;
    ListEntry_t *qtr = NULL;
    int newTime = 0, oldTime = 0, diffTime = 0;

    while (1)
    {
        /* 处理有线程退出的情况 */
        list_for_each_safe (ptr, qtr, &pt_exitlist)
        {
            tcb = list_container_of(ptr, ThreadCB, list);
            if (tcb->tid == tid)
                return tcb;
        }

        /* 管理休眠的线程 */
        oldTime = newTime;
        newTime = gettime();
        diffTime = newTime - oldTime;
        if (diffTime)
        {
            list_for_each_safe (ptr, qtr, &pt_sleeplist)
            {
                tcb = list_container_of(ptr, ThreadCB, list);
                if (tcb->sleep > diffTime)
                {
                    tcb->sleep -= diffTime;
                    continue;
                }

                list_del_init(&tcb->list);
                tcb->stat = READY;
                tcb->sleep = 0;
                list_add_after(&pt_readylist, &tcb->list);
            }
        }

        /* 管理就绪的线程 */
        list_for_each_safe (ptr, qtr, &pt_readylist)
        {
            tcb = list_container_of(ptr, ThreadCB, list);
            tcb->stat = RUNNING;

            /* 切换到新的进程 */
            currTCB = tcb;
            thread_switch(&idleTCB->context, &currTCB->context);
            currTCB = idleTCB;
        }
    }

    return tcb;
}
/* 所有线程的入口 */
static void pthread_entry (void)
{
    void *ret = NULL;

    ret = currTCB->entry(currTCB->arg);
    pthread_exit(ret);
}
/* 申请已初始化的线程控制块 */
static ThreadCB *pthread_alloc (void)
{
    void *stack = NULL;
    ThreadCB *tcb = NULL;

    stack = malloc(1024);
    if (stack == NULL)
        return NULL;
    
    tcb = malloc(sizeof(ThreadCB));
    if (tcb == NULL)
    {
        free(stack);
        return NULL;
    }
    memset(tcb, 0, sizeof(ThreadCB));

    tcb->context.ra = (uint64)pthread_entry;
    tcb->context.sp = (uint64)(stack + 1024);
    tcb->stack      = stack;
    tcb->tid        = tid_token++;
    tcb->stat       = IDLE;
    tcb->exitval    = NULL;

    list_init(&tcb->list);
    return tcb;
}
/* 释放已申请的线程控制块 */
static void pthread_free (ThreadCB *tcb)
{
    if (tcb == NULL)
        return;
    
    list_del_init(&tcb->list);
    free(tcb->stack);
    free(tcb);
}

/* 初始化整个线程管理模块 */ 
void init_pthread (void)
{
    tid_token = 0;

    /* 初始化线程模块内部的管理链表 */
    list_init(&pt_readylist);
    list_init(&pt_sleeplist);
    list_init(&pt_exitlist);

    /* 初始化空闲线程的控制块 */
    idleTCB = pthread_alloc();
    idleTCB->entry = NULL;
    idleTCB->attr  = NULL;
    idleTCB->arg   = (void *)"idle";

    free(idleTCB->stack);

    /* 设置主线程 */
    currTCB = idleTCB;
}


/* 创建线程 
 * ( 目前不接受线程属性的设置功能 )
 */
int pthread_create (pthread_t *thread, const pthread_attr_t *attr, 
        void *(*start_routine)(void *), void *arg)
{
    ThreadCB *tcb = NULL;

    if ((thread == NULL) || (start_routine == NULL))
        return -1;

    tcb = pthread_alloc();
    if (tcb == NULL)
        return -1;

    /* 返回当前线程的 ID */
    *thread = tcb->tid;

    /* 记录要写入线程控制块的信息*/
    tcb->entry      = start_routine;
    tcb->attr       = attr;
    tcb->arg        = arg;

    list_add (&pt_readylist, &tcb->list);
    return 0;
}

/* 退出线程 */
int pthread_exit (void *retval)
{
    /* 移动线程所属的链表 */
    list_del_init(&currTCB->list);
    list_add(&pt_exitlist, &currTCB->list);

    currTCB->stat = EXITING;
    currTCB->exitval = retval;    

    /* 切换到空闲线程 */
    thread_switch(&currTCB->context, &idleTCB->context);
    return 0;
}

/* 线程休眠 */
int pthread_sleep(int ms)
{
    if (ms == 0)
        return -1;

    /* 移动线程所属的链表 */
    list_del_init(&currTCB->list);
    list_add(&pt_sleeplist, &currTCB->list);

    currTCB->stat = SLEEPING;
    currTCB->sleep = ms;

    /* 切换到空闲线程 */
    thread_switch(&currTCB->context, &idleTCB->context);
    return 0;
}

/* 等待指定线程退出 */
int pthread_join(pthread_t thread, void **retval)
{
    ThreadCB *tcb = NULL;

    while(tcb->tid != thread)
    {
        tcb = pthread_scheduler(thread);
    }

    if (retval != NULL)
        *retval = tcb->exitval;

    pthread_free(tcb);
    return thread;
}
