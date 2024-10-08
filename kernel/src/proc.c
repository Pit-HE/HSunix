/* 
 * 操作系统进程管理模块
 */
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "timer.h"
#include "fs.h"
#include "pcb.h"
#include "proc.h"
#include "kstring.h"


/**********************************************/
/* PID 令牌：
用于记录每一次进程创建时所分配的PID，该变量自动递增
*/
uint64          kPidToken = 1;
/* cpu 管理链表：
记录每一个 cpu 在运行过程中的信息 (当前系统未适配多核）
*/
struct CpuCB    kCpusList[NCPU];
/* 进程注销链表：
记录被杀死(kill)的进程，由死亡进程回收器对其进行释放
*/
ListEntry_t     kUnregistList;
/* 进程注册链表：
记录系统从运行到结束过程中，创建的所有存活的进程
*/
ListEntry_t     kRegistList;
/* 进程就绪链表：
记录所有就绪的等待切换执行的进程
*/
ListEntry_t     kReadyList;
/* 进程挂起链表：
记录所有在等待指定世界的进程
*/
ListEntry_t     kPendList;
/* 记录 init 进程控制块 */
struct ProcCB  *kInitPCB = NULL;
/* 记录 idle 进程控制块 */
struct ProcCB  *kIdlePCB = NULL;

/***********************************************
 *  Process file public library function
*/
/* 设置占用当前 cpu 使用权的进程 */
void setCpuCB (struct ProcCB *pcb)
{
    kCpusList[getCpuID()].proc = pcb;
}
/* 获取当前占用 cpu 使用权的进程 */
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
    if (pcb->state == READY)
        return;
    pcb->state = READY;

    if (pcb != curPcb)
    {
        kDISABLE_INTERRUPT();
        list_del_init(&pcb->list);
        list_add(&kReadyList, &pcb->list);
        kENABLE_INTERRUPT();
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
    /* 实现内核线程的时间片轮转调度功能 */
    if (kReadyList.next != &kReadyList)
    {
        ptr = kReadyList.next;
        list_del_init(ptr);
        list_add_before(&kReadyList, ptr);
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
    if (cpu->intrOffNest != 0)
        ErrPrint("fail: do_switch isr nest!\r\n");
    if (pcb->state == RUNNING)
        ErrPrint("fail: do_switch process is running!\r\n");
    // if (intr_get())
    //     ErrPrint("fail: do_switch isr no close!\r\n");

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

    /* 变量所有被挂起的进程 */
    list_for_each_safe(ptr, qtr, &kPendList)
    {
        /* 查找等待指定事件的进程 */
        pcb = list_container_of(ptr, struct ProcCB, list);
        if (pcb->pendObj == obj)
        {
            /* 若该进程存在，则将其添加到就绪链表 */
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

    stack = (char *)alloc_page();
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
    /* 确保新进程返回用户空间后，从指定位置开始继续执行 */
    kmemcpy(newPcb->trapFrame, curPcb->trapFrame, PGSIZE);
    newPcb->memSize = curPcb->memSize;
    /* 设置新进程的返回值 */
    newPcb->trapFrame->a0 = 0;
    newPcb->parent = curPcb;
    kENABLE_INTERRUPT();

    /* 唤醒新进程，将其添加到就绪队列等待调度 */
    proc_wakeup(newPcb);

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

                /* 释放该进程占用的资源 */
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
        ErrPrint("exit error process !\r\n");

    /* 将目标的所有子进程挂到 init 进程上 */
    proc_freechild(pcb);

    /* 修改进程的状态 */
    kDISABLE_INTERRUPT();
    pcb->exitState = state;
    pcb->state = EXITING;
    kENABLE_INTERRUPT();

    /* 唤醒父进程 */
    do_resume(pcb->parent);
    do_switch();
}

/* 杀死指定 id 的进程 */
int do_kill (int pid)
{
    struct ProcCB *pcb = NULL;

    /* 获取要操作的进程对象 */
    pcb = pcb_lookup(pid);
    if ((pcb != NULL) && (pcb->state != EXITING))
    {
        /* 将进程添加到注销链表 */
        kDISABLE_INTERRUPT();
        pcb->killState = 1;
        list_del_init(&pcb->regist);
        list_add(&kUnregistList, &pcb->regist);
        kENABLE_INTERRUPT();

        /* 唤醒该进程，让其执行完最后的操作 */
        proc_wakeup(pcb);
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

        /* 创建新定时器，
        用于指定时长后唤醒当前进程 */
        kDISABLE_INTERRUPT();
        timer = timer_add(pcb, ms);
        kENABLE_INTERRUPT();

        /* 进程进入休眠，放弃 CPU 使用权 */
        do_switch();

        /* 进程从休眠中唤醒，
        删除软件定时器，释放其占用的资源 */
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

    /* 申请新的任务控制块 */
    pcb = pcb_alloc();
    if (pcb == NULL)
        return NULL;
    
    /* 申请内核栈地址 */
    stack = (char *)alloc_page();
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

    return pcb;
}

/* 销毁创建的内核线程 */
void destroy_kthread (struct ProcCB *pcb)
{
    vfs_pcbdeinit(pcb);
    free_page((void *)pcb->stackAddr);
    pcb_free(pcb);
}

/* 初始化当前用于测试的指定进程 */
void init_proc (void)
{
    list_init(&kRegistList);
    list_init(&kReadyList);
    list_init(&kPendList);
    list_init(&kUnregistList);

    /* Init 进程，用户空间的第一个进程 */
    kInitPCB = create_kthread("init", init_main);
    vfs_pcbInit(kInitPCB, "/");
    proc_wakeup(kInitPCB);
	kprintf ("'init' kernel thread create complete !\r\n");

    /* Idle 进程 */
    kIdlePCB = create_kthread("idle", idle_main);
    vfs_pcbInit(kIdlePCB, "/");
    proc_wakeup(kIdlePCB);
	kprintf ("'idle' kernel thread create complete !\r\n");

    /* 设置当前 CPU 的默认进程 */
    setCpuCB(kIdlePCB);
	kprintf ("init_proc complete !\r\n");
}
