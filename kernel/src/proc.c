
#include "defs.h"
#include "param.h"
#include "memlayout.h"


/**********************************************/
uint64          kPidToken = 1;
struct CpuCB    kCpusList[NCPU];
ListEntry_t     kUnregistList;
ListEntry_t     kRegistList;
ListEntry_t     kReadyList;
ListEntry_t     kPendList;
struct ProcCB  *kInitPCB = NULL;
struct ProcCB  *kIdlePCB = NULL;

/***********************************************
 *  Process file public library function
*/
void setCpuCB (struct ProcCB *pcb)
{
    kCpusList[getCpuID()].proc = pcb;
}
struct CpuCB *getCpuCB (void)
{
    return &kCpusList[getCpuID()];
}


/* 申请可用的进程 PID */
int proc_applypid (void)
{
    return (kPidToken++);
}

/* 唤醒指定的进程 */
void proc_wakeup (struct ProcCB *pcb)
{
    struct ProcCB *curPcb = getProcCB();

    if (pcb == NULL)
        return;

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
void proc_freechild (struct ProcCB *pcb)
{
    struct ProcCB *childPcb = NULL;
    ListEntry_t *ptr = NULL;
    ListEntry_t *qtr = NULL;

    /* 确定当前进程是否有子进程 */
    list_for_each_safe (ptr, qtr, &kRegistList)
    {
        childPcb = list_container_of(ptr, struct ProcCB, regist);

	    /* 将子进程挂载到 init 进程上 */
        if (childPcb->parent == pcb)
        {
            childPcb->parent = kInitPCB;

            if (childPcb->state == EXITING)
                do_resume(kInitPCB);
        }
    }
}

/* 获取进程的死亡状态 */
int proc_killstate (struct ProcCB *pcb)
{
    return pcb->killState;
}

/* 每个 fork 后的进程第一次执行的函数入口 
 * (当任务创建后，通过 kswitch_to 到此函数)
 */
void proc_entry (void)
{
    kENABLE_INTERRUPT();

    trap_userret();
}

/* 就绪进程调度器 */
void do_scheduler (void)
{
    ListEntry_t *ptr = NULL;
    struct CpuCB *cpu = NULL;
    struct ProcCB *pcb = NULL;

    intr_on();
    if (kReadyList.next == &kReadyList)
        return;

    /* search all ready member of list */
    list_for_each (ptr, &kReadyList)
    {
        cpu = getCpuCB();
        pcb = list_container_of(ptr, struct ProcCB, list);

        if ((pcb->state == READY) && (pcb != cpu->proc))
        {
            kDISABLE_INTERRUPT();
            pcb->state = RUNNING;
            cpu->proc = pcb;

            kswitch_to(&cpu->context, &pcb->context);

            cpu->proc = kIdlePCB;
            kENABLE_INTERRUPT();
            break;
        }
    }
}

/* 死亡进程回收器 */
void do_defuncter (void)
{
    ListEntry_t *ptr = NULL;
    ListEntry_t *qtr = NULL;
    struct ProcCB *pcb = NULL;

    if (kUnregistList.next == &kUnregistList)
        return;

    /* 遍历死亡进程链表 */
    list_for_each_safe(ptr, qtr, &kUnregistList)
    {
        pcb = list_container_of(ptr, struct ProcCB, regist);

        /* 处理当前死亡进程的子进程 */
        proc_freechild(pcb);

        /* 释放该死亡进程占用的所有资源 */
        destroy_kthread(pcb);
    }
}

/* 执行进程切换的接口 */
void do_switch (void)
{
    // int state;
    struct CpuCB  *cpu = getCpuCB();
    struct ProcCB *pcb = getProcCB();

    /* TODO: */
    // if (cpu->intrOffNest != 0)
    //     kErrPrintf("fail: do_switch isr nest!\r\n");
    if (pcb->state == RUNNING)
        kErrPrintf("fail: do_switch process is running!\r\n");
    // if (intr_get())
    //     kErrPrintf("fail: do_switch isr no close!\r\n");

    /* 记录进程切换前的中断状态，并执行切换 */
    kDISABLE_INTERRUPT();
    // state = cpu->intrOldState;
    kswitch_to(&pcb->context, &cpu->context);
    // cpu->intrOldState = state;
    kENABLE_INTERRUPT();
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
    struct ProcCB *pcb =getProcCB();

    if (obj == NULL)
        return;

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
    struct ProcCB *pcb = NULL;
    ListEntry_t *ptr = NULL;
    ListEntry_t *qtr = NULL;

    if (obj == NULL)
        return;

    list_for_each_safe(ptr, qtr, &kPendList)
    {
        pcb = list_container_of(ptr, struct ProcCB, list);
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
    int i;
    char *stack = NULL;
    struct ProcCB *newPcb = NULL;
    struct ProcCB *curPcb = getProcCB();

    stack = (char *)kallocPhyPage;
    if (stack == NULL)
        return -1;

    /* 创建内核线程 */
    newPcb = create_kthread(curPcb->name, proc_entry);
    if (newPcb == NULL)
        return -1;

    /* 完成两个进程间页表空间的复制 */
    if (uvm_copy(newPcb->pgtab, curPcb->pgtab, curPcb->memSize, TRUE) < 0)
    {
        destroy_kthread(newPcb);
        return -1;
    }

    /* 设置新进程文件系统相关的内容 */
    vfs_pcbInit(newPcb, curPcb->cwd);
    for (i=0; i<curPcb->fdCnt; i++)
    {
        if (curPcb->fdTab[i])
            newPcb->fdTab[i] = fd_copy(curPcb->fdTab[i]);
    }

    /* 完成进程控制块相关信息的拷贝 */
    kDISABLE_INTERRUPT();
    kmemcpy(newPcb->trapFrame, curPcb->trapFrame, PGSIZE);
    kmemcpy(&newPcb->context, &curPcb->context, sizeof(struct Context));
    kmemcpy((void*)newPcb->stackAddr, (void*)curPcb->stackAddr, curPcb->stackSize);

    newPcb->context.sp = (uint64)(stack + (curPcb->context.sp - curPcb->stackAddr));
    newPcb->memSize = curPcb->memSize;
    newPcb->trapFrame->a0 = 0;
    newPcb->parent = curPcb;

    proc_wakeup(newPcb);
    kENABLE_INTERRUPT();

    return newPcb->pid;
}

/* 等待处理退出的子进程 */
int do_wait (int *code)
{
    int pid = -1;
    ListEntry_t *ptr = NULL;
    ListEntry_t *qtr = NULL;
    struct ProcCB *curPcb = NULL;
    struct ProcCB *childPcb = NULL;

    curPcb = getProcCB();
    while(1)
    {
        /* 遍历当前内核里的所有进程控制块链表 */
        list_for_each_safe(ptr, qtr, &kRegistList)
        {
            childPcb = list_container_of(ptr, struct ProcCB, regist);

            /* 查找需要退出的子进程 */
            if ((childPcb->parent == curPcb) &&
                (childPcb->state == EXITING))
            {
		        /* 处理正在退出的子进程 */
                if (code != NULL)
                    *code = childPcb->exitState;

                pid = childPcb->pid;
                destroy_kthread(childPcb);
                goto _exit_wait;
            }
            /* 非法操作：释放当前进程 */
            if (proc_killstate(curPcb))
                goto _exit_wait;
        }
        do_suspend(curPcb);
    }

/* 返回已退出的子进程 ID */
 _exit_wait:
    return pid;
}

/* 退出当前进程 */
void do_exit (int state)
{
    struct ProcCB *pcb = NULL;

    pcb = getProcCB();
    if ((pcb == kInitPCB) || (pcb == kIdlePCB))
        kErrPrintf("exit error process !\r\n");

    /* 将子进程释放到 init 进程上 */
    proc_freechild(pcb);

    pcb->exitState = state;
    pcb->state = EXITING;

    /* 唤醒父进程 */
    do_resume(pcb->parent);
    do_switch();
}

/* 杀死指定 id 的进程 */
int do_kill (int pid)
{
    struct ProcCB *pcb = NULL;

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
    struct Timer *timer = NULL;
    struct ProcCB *pcb = NULL;

    if (ms != 0)
    {
        pcb = getProcCB();
        kDISABLE_INTERRUPT();
        timer = timer_add(pcb, ms);
        kENABLE_INTERRUPT();

        do_switch();

        kDISABLE_INTERRUPT();
        timer_del(timer);
        kENABLE_INTERRUPT();
    }

    return 0;
}

/* 创建内核线程 */
struct ProcCB *create_kthread (char *name, void(*entry)(void))
{
    char *stack = NULL;
    struct ProcCB *pcb = NULL;

    pcb = pcb_alloc();
    if (pcb == NULL)
        return NULL;
    stack = (char *)kallocPhyPage();
    if (stack == NULL)
    {
        pcb_free(pcb);
        return NULL;
    }

    /* 添加信息到进程的控制块 */
    kstrcpy(pcb->name, name);
    pcb->context.ra = (uint64)entry;
    pcb->stackAddr  = (uint64)stack;
    pcb->stackSize  = (uint64)PGSIZE;
    pcb->context.sp = (uint64)(stack + PGSIZE);

    vfs_pcbInit(pcb, "/");

    return pcb;
}

/* 销毁创建的内核线程 */
void destroy_kthread (struct ProcCB *pcb)
{
    vfs_pcbdeinit(pcb);
    kfreePhyPage((void*)pcb->stackAddr);
    pcb_free(pcb);
}

/* 初始化当前用于测试的指定进程 */
void init_proc (void)
{
    list_init(&kRegistList);
    list_init(&kReadyList);
    list_init(&kPendList);
    list_init(&kUnregistList);

    /* Init processs */
    kInitPCB = create_kthread("init", init_main);
    proc_wakeup(kInitPCB);

    /* Idle processs */
    kIdlePCB = create_kthread("idle", idle_main);
    proc_wakeup(kIdlePCB);

    /* 命令行交互进程 (仅用于内核开发阶段的测试进程) */
    proc_wakeup(create_kthread("test", test_main));

    /* 设置当前 CPU 的默认进程 */
    setCpuCB(kIdlePCB);
}
