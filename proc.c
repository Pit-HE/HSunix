
#include "defs.h"
#include "param.h"
#include "memlayout.h"


/**********************************************/
uint64       kPidToken = 1;
CpuCB        kCpusList[NCPU];
ListEntry_t  kUnregistList;
ListEntry_t  kRegistList;
ListEntry_t  kReadyList;
ListEntry_t  kPendList;
ProcCB      *kInitProcCB;
ProcCB      *kIdleProcCB;

/***********************************************
 *  Process file public library function
*/
void setCpuCB (ProcCB *pcb)
{
    kCpusList[getCpuID()].proc = pcb;
}
CpuCB *getCpuCB (void)
{
    return &kCpusList[getCpuID()];
}


/* 申请可用的进程 PID */
int proc_applypid (void)
{
    return (kPidToken++);
}

/* 唤醒指定的进程 */
void proc_wakeup (ProcCB *pcb)
{
    ProcCB *curPcb;

    curPcb = getProcCB();
    if (pcb->state != READY)
    {
        pcb->state = READY;

        if (pcb != curPcb)
        {
            kDISABLE_INTERRUPT();
            list_del_init(&pcb->list);
            list_add(&kReadyList, &pcb->list);
            kENABLE_INTERRUPT();
        }
    }
}

/* 释放进程的所有子进程到 init 进程 */
void proc_freechild (ProcCB *pcb)
{
    ProcCB *childPcb;
    ListEntry_t *ptr, *qtr;

    /* 确定当前进程是否有子进程 */
    list_for_each_safe (ptr, qtr, &kRegistList)
    {
        childPcb = list_container_of(ptr, ProcCB, regist);

	    /* 将子进程挂载到 init 进程上 */
        if (childPcb->parent == pcb)
        {
            childPcb->parent = kInitProcCB;

            if (childPcb->state == EXITING)
                do_resume(kInitProcCB);
        }
    }
}

/* 获取进程的死亡状态 */
int proc_killstate (ProcCB *pcb)
{
    return pcb->killState;
}

/* 就绪进程调度器 */
void do_scheduler (void)
{
    CpuCB *cpu = NULL;
    ProcCB *pcb = NULL;
    ListEntry_t *ptr = NULL;

    intr_on();
    if (kReadyList.next == &kReadyList)
        return;

    /* search all ready member of list */
    list_for_each (ptr, &kReadyList)
    {
        cpu = getCpuCB();
        pcb = list_container_of(ptr, ProcCB, list);

        if ((pcb->state == READY) && (pcb != cpu->proc))
        {
            kDISABLE_INTERRUPT();
            pcb->state = RUNNING;
            cpu->proc = pcb;
            kENABLE_INTERRUPT();

            switch_to(&cpu->context, &pcb->context);

            cpu->proc = kIdleProcCB;
            break;
        }
    }
}

/* 死亡进程回收器 */
void do_defuncter (void)
{
    ProcCB *pcb;
    ListEntry_t *ptr, *qtr;

    if (kUnregistList.next == &kUnregistList)
        return;

    /* 遍历死亡进程链表 */
    list_for_each_safe(ptr, qtr, &kUnregistList)
    {
        pcb = list_container_of(ptr, ProcCB, regist);

        kDISABLE_INTERRUPT();
        /* 处理当前死亡进程的子进程 */
        proc_freechild(pcb);

        /* 释放该死亡进程占用的所有资源 */
        pcb_free(pcb);
        kENABLE_INTERRUPT();
    }
}

/* 执行进程切换的接口 */
void do_switch (void)
{
    int         state;
    CpuCB     *cpu = getCpuCB();
    ProcCB    *pcb = getProcCB();

    if (cpu->intrOffNest != 0)
        kError(eSVC_Process, E_INTERRUPT);
    if (pcb->state == RUNNING)
        kError(eSVC_Process, E_PROCESS);
    if (intr_get())
        kError(eSVC_Process, E_INTERRUPT);

    /* 记录进程切换前的中断状态，并执行切换 */
    state = cpu->intrOldState;
    switch_to(&pcb->context, &cpu->context);
    cpu->intrOldState = state;
}

/* 外部接口，释放当前进程的 cpu 使用权 */
void do_yield (void)
{
    kDISABLE_INTERRUPT();
    getProcCB()->state = READY;
    kENABLE_INTERRUPT();

    do_switch();
}

/* 将当前进程挂起到指定的对象 */
void do_suspend (void *obj)
{
    ProcCB *pcb = NULL;

    pcb = getProcCB();

    kDISABLE_INTERRUPT();
    pcb->pendObj = obj;
    pcb->state = SUSPEND;
    list_del_init(&pcb->list);
    list_add(&kPendList, &pcb->list);
    kENABLE_INTERRUPT();

    do_switch();

    pcb->pendObj = NULL;
}

/* 从指定对象恢复被挂起的进程 */
void do_resume (void *obj)
{
    ProcCB        *pcb = NULL;
    ListEntry_t    *ptr, *qtr;

    list_for_each_safe(ptr, qtr, &kPendList)
    {
        pcb = list_container_of(ptr, ProcCB, list);
        if (pcb->pendObj == obj)
        {
            kDISABLE_INTERRUPT();
            pcb->state = READY;
            list_del_init(&pcb->list);
            list_add(&kReadyList, &pcb->list);
            kENABLE_INTERRUPT();
        }
    }
}

/* 复制当前进程，生成新的进程 */
int do_fork (void)
{
    int pid = -1;
    char *stack;
    ProcCB *newPcb;
    ProcCB *curPcb = getProcCB();

    stack = (char *)kallocPhyPage;
    if (stack == NULL)
        goto _exit_fork;

    /* 申请新的进程控制块 */
    newPcb = pcb_alloc();
    if (newPcb == NULL)
    {
        kfree(stack);
        goto _exit_fork;
    }

    /* 完成两个进程间页表空间的复制 */
    if (uvm_copy(newPcb->pageTab, curPcb->pageTab, curPcb->memSize, TRUE) < 0)
    {
        kfree(stack);
        pcb_free(newPcb);
        uvm_destroy(newPcb->pageTab);
        goto _exit_fork;
    }

    /* 设置新进程文件系统相关的内容 */
    vfs_pcbInit(newPcb, curPcb->cwd);

    kDISABLE_INTERRUPT();
    newPcb->stackSize = PGSIZE;
    newPcb->memSize = curPcb->memSize;
    newPcb->stackAddr = (uint64)stack;
    newPcb->trapFrame->a0 = 0;
    newPcb->parent = curPcb;

    kstrcpy(newPcb->name, curPcb->name);
    kmemcpy(&newPcb->context, &curPcb->context, sizeof(Context));
    kmemcpy((void*)newPcb->stackAddr, (void*)curPcb->stackAddr, curPcb->stackSize);

    newPcb->context.sp = (uint64)(stack + (curPcb->context.sp - curPcb->stackAddr));
    proc_wakeup(newPcb);

    pid = newPcb->pid;
    kENABLE_INTERRUPT();

 _exit_fork:
    return pid;
}

/* 等待处理退出的子进程 */
int do_wait (int *code)
{
    int pid = -1;
    ProcCB *childPcb, *curPcb;
    ListEntry_t *ptr, *qtr;

    curPcb = getProcCB();
    while(1)
    {
        list_for_each_safe(ptr, qtr, &kRegistList)
        {
            childPcb = list_container_of(ptr, ProcCB, regist);

            /* 查找需要退出的子进程 */
            if ((childPcb->parent == curPcb) &&
                (childPcb->state == EXITING))
            {
		        /* 处理正在退出的子进程 */
                if (code != NULL)
                    *code = childPcb->exitState;

                pid = childPcb->pid;
                pcb_free(childPcb);
                goto _exit_wait;
            }
            if (proc_killstate(curPcb))
            {
                pid = -1;
                goto _exit_wait;
            }
        }
        do_suspend(curPcb);
    }

 _exit_wait:
    return pid;
}

/* 退出当前进程 */
void do_exit (int state)
{
    ProcCB *curPcb;

    curPcb = getProcCB();
    if ((curPcb == kInitProcCB) ||
        (curPcb == kIdleProcCB))
        kError(eSVC_Process,E_DANGER);

    /* 确定当前进程是否有子进程 */
    proc_freechild(curPcb);

    curPcb->exitState = state;
    curPcb->state = EXITING;

    do_resume(curPcb->parent);
    do_switch();
}

/* 杀死指定 id 的进程 */
int do_kill (int pid)
{
    ProcCB *pcb;

    pcb = pcb_lookup(pid);
    if ((pcb != NULL) && (pcb->state != EXITING))
    {
        kDISABLE_INTERRUPT();
        pcb->killState = 1;
        list_del_init(&pcb->regist);
        list_add(&kUnregistList, &pcb->regist);

        proc_wakeup(pcb);
        kENABLE_INTERRUPT();
        return 0;
    }
    return -1;
}

/* 让进程休眠指定的时长后唤醒 */
int do_sleep (int ms)
{
    timer_t *timer = NULL;
    ProcCB *pcb = NULL;

    if (ms != 0)
    {
        pcb = getProcCB();
        timer = timer_add(pcb, ms);

        do_switch();

        timer_del(timer);
    }

    return 0;
}

/* 创建内核线程 */
ProcCB *do_kthread (char *name, void(*thread)(void))
{
    char *stack = NULL;
    ProcCB *pcb = NULL;

    pcb = pcb_alloc();
    stack = (char *)kallocPhyPage();

    /* 添加信息到进程的控制块 */
    kstrcpy(pcb->name, name);
    pcb->context.ra = (uint64)thread;
    pcb->stackAddr  = (uint64)stack;
    pcb->stackSize  = (uint64)PGSIZE;
    pcb->context.sp = (uint64)(stack + PGSIZE);

    vfs_pcbInit(pcb, "/");

    return pcb;
}

/* 初始化当前用于测试的指定进程 */
void init_proc (void)
{
    list_init(&kRegistList);
    list_init(&kReadyList);
    list_init(&kPendList);
    list_init(&kUnregistList);

    /* Init processs */
    kInitProcCB = do_kthread("init", init_main);
    proc_wakeup(kInitProcCB);

    /* Idle processs */
    kIdleProcCB = do_kthread("idle", idle_main);
    proc_wakeup(kIdleProcCB);


    /* 只要注释以下的进程创建，
     * 便能关闭用户模式切换的测试功能, 之后系统正常工作
     */
    // proc_wakeup(do_kthread("user", user_main));


    /* 设置当前 CPU 的默认进程 */
    setCpuCB(kIdleProcCB);
}
