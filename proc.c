
#include "defs.h"
#include "param.h"
#include "memlayout.h"



/**********************************************/
static uint64           kPidToken = 1;
static CpuCB_t          kCpusList[NCPU];
static list_entry_t     kUnregistList;
static list_entry_t     kRegistList;
static list_entry_t     kReadyList;
static list_entry_t     kPendList;
static ProcCB_t        *kInitProcCB;
static ProcCB_t        *kIdleProcCB;

/***********************************************
 *  Process file public library function
*/
#if 1
void setCpuCB (ProcCB_t *pcb)
{
    kCpusList[getCpuID()].proc = pcb;
}
CpuCB_t *getCpuCB (void)
{
    return &kCpusList[getCpuID()];
}
ProcCB_t *getProcCB (void)
{
    return getCpuCB()->proc;
}
void wakeProcCB (ProcCB_t *pcb)
{
    ProcCB_t *curPcb;

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
void freeChildProcCB (ProcCB_t *pcb)
{
    ProcCB_t *childPcb;
    list_entry_t *ptr, *qtr;

    /* 确定当前进程是否有子进程 */ 
    list_for_each_safe (ptr, qtr, &kRegistList)
    {
        childPcb = list_container_of(ptr, ProcCB_t, regist);

	    /* 将子进程挂载到 init 进程上 */
        if (childPcb->parent == pcb)
        {
            childPcb->parent = kInitProcCB;

            if (childPcb->state == EXITING)
                do_resume(kInitProcCB);
        }
    }
}
void dumpProcCB (void)
{
    static char *states[7] =
    {
        [IDLE]      "idle",
        [USED]      "used",
        [SUSPEND]   "suspend",
        [SLEEPING]  "sleep",
        [READY]     "ready",
        [RUNNING]   "run",
        [EXITING]   "exit"
    };
    list_entry_t *ptr;
    ProcCB_t *pcb;
    char *state;

    kprintf("\n");

    /* 遍历进程控制块数组 */
    list_for_each(ptr, &kRegistList)
    {
        pcb = list_container_of(ptr, ProcCB_t, regist);

        /* 寻找非空闲的进程 */
        if (pcb->state == IDLE)
            continue;

        /* 获取进程的状态 */
        if((pcb->state >= 0) && (pcb->state < 7) && (states[pcb->state]))
            state = states[pcb->state];
        else
            state = "???";

        /* 打印进程的状态 */
        kprintf("%d %s %s", pcb->pid, state, pcb->name);
        kprintf("\n");
    }
}
int allocPid (void)
{
    return (kPidToken++);
}
ProcCB_t *allocProcCB (void)
{
    ProcCB_t *pcb;

    /* 申请任务控制块的内存空间 */
    pcb = (ProcCB_t *)kalloc(sizeof(ProcCB_t));
    if (pcb == NULL)
        goto _exit_allocProcCB;
    memset(pcb, 0, sizeof(ProcCB_t));

    /* 申请存放进程trap信息的内存空间 */
    pcb->trapFrame = (Trapframe_t *)kalloc(sizeof(Trapframe_t));
    if (pcb->trapFrame == NULL)
    {
        kfree(pcb);
        pcb = NULL;
        goto _exit_allocProcCB;
    }
    memset(pcb->trapFrame, 0, sizeof(Trapframe_t));

    /* 为进程申请在用户模式中使用的栈 */
    pcb->trapFrame->sp = (uint64)kalloc(2048);
    if (pcb->trapFrame->sp == 0)
    {
        kfree (pcb->trapFrame);
        kfree (pcb);
        pcb = NULL;
        goto _exit_allocProcCB;
    }

    // pcb->pageTab = uvm_create();
    // if (pcb->pageTab == NULL)
    // {
    //     kfree (pcb->trapFrame->sp);
    //     kfree (pcb->trapFrame);
    //     kfree (pcb);
    //     pcb = NULL;
    //     goto _exit_allocProcCB;
    // }

    memset(&pcb->context, 0, sizeof(Context_t));

    pcb->pid = allocPid();
    pcb->state = USED;

    list_init(&pcb->list);
    list_init(&pcb->regist);
    list_add(&kRegistList, &pcb->regist);

_exit_allocProcCB:
    return pcb;
}
int freeProcCB (ProcCB_t *pcb)
{
    if (pcb == NULL)
        return -1;

    kDISABLE_INTERRUPT();
    /* 移除进程所挂载的链表 */
    list_del_init(&pcb->regist);
    list_del_init(&pcb->list);

    /* 释放进程申请的栈帧空间 */
    if (pcb->trapFrame)
        kfree(pcb->trapFrame);
    pcb->trapFrame = NULL;

    /* 释放进程所占用的所有物理内存页 */
    // uvm_free(pcb->pageTab, 0, pcb->memSize);

    /* 释放进程申请的页表 */
    // if (pcb->pageTab)
    //     uvm_destroy(pcb->pageTab);
    pcb->pageTab = NULL;

    /* 释放进程控制块的资源 */
    kfree(pcb);
    kENABLE_INTERRUPT();

    return 1;
}
ProcCB_t *findProcCB (int pid)
{
    ProcCB_t *pcb = NULL;
    list_entry_t *ptr = NULL;

    if ((kPidToken < pid) && (pid < 0))
        goto exit_findProcCB;
    
    list_for_each(ptr, &kRegistList)
    {
        pcb = list_container_of(ptr, ProcCB_t, regist);
        if (pcb->pid == pid)
            goto exit_findProcCB;
    }
exit_findProcCB:
    return pcb;
}
#endif



void proc_init (void)
{
    char *stack;
    ProcCB_t *pcb;

    list_init(&kRegistList);
    list_init(&kReadyList);
    list_init(&kPendList);
    list_init(&kUnregistList);

    /* Init processs */
    kInitProcCB = allocProcCB();
    stack = (char *)kalloc(2048);
    memset(stack, 0, 2048);
    strcpy(kInitProcCB->name, "init");
    kInitProcCB->context.ra = (uint64)init_main;
    kInitProcCB->stackAddr  = (uint64)stack;
    kInitProcCB->stackSize  = (uint64)2048;
    kInitProcCB->context.sp = (uint64)(stack + 2048);
    wakeProcCB(kInitProcCB);

    /* Idle processs */
    kIdleProcCB = allocProcCB();
    stack = (char *)kalloc(2048);
    memset(stack, 0, 2048);
    strcpy(kIdleProcCB->name, "idle");
    kIdleProcCB->context.ra = (uint64)idle_main;
    kIdleProcCB->stackAddr  = (uint64)stack;
    kIdleProcCB->stackSize  = (uint64)2048;
    kIdleProcCB->context.sp = (uint64)(stack + 2048);
    wakeProcCB(kIdleProcCB);
    setCpuCB(kIdleProcCB);

    /* test process */
    pcb = allocProcCB();
    stack = (char *)kalloc(2048);
    memset(stack, 0, 2048);
    strcpy(pcb->name, "test");
    pcb->context.ra = (uint64)test_main;
    pcb->stackAddr  = (uint64)stack;
    pcb->stackSize  = (uint64)2048;
    pcb->context.sp = (uint64)(stack + 2048);
    wakeProcCB(pcb);

    /* 只要注释以下的进程创建，
     * 便能关闭用户模式切换的测试功能,之后系统正常工作
     */
    pcb = allocProcCB();
    stack = (char *)kalloc(2048);
    memset(stack, 0, 2048);
    strcpy(pcb->name, "user");
    void user_ret (void);
    pcb->context.ra = (uint64)user_ret;
    pcb->stackAddr  = (uint64)stack;
    pcb->stackSize  = (uint64)2048;
    pcb->context.sp = (uint64)(stack + 2048);
    /* 设置进程在用户模式下执行的函数 */
    void user_processEntry (void);
    pcb->trapFrame->epc = (uint64)user_processEntry;
    wakeProcCB(pcb);
}

/* 就绪进程调度器 */
void scheduler (void)
{
    CpuCB_t         *cpu;
    ProcCB_t        *pcb;
    list_entry_t    *ptr;

    intr_on();
    if (kReadyList.next == &kReadyList)
        return;

    /* search all ready member of list */
    list_for_each (ptr, &kReadyList)
    {
        cpu = getCpuCB();
        pcb = list_container_of(ptr, ProcCB_t, list);

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
void defuncter (void)
{
    ProcCB_t *pcb;
    list_entry_t *ptr, *qtr;

    if (kUnregistList.next == &kUnregistList)
        return;

    /* 遍历死亡进程链表 */
    list_for_each_safe(ptr, qtr, &kUnregistList)
    {
        pcb = list_container_of(ptr, ProcCB_t, regist);

        kDISABLE_INTERRUPT();
        /* 处理当前死亡进程的子进程 */
        freeChildProcCB(pcb);

        /* 释放该死亡进程占用的所有资源 */
        freeProcCB(pcb);
        kENABLE_INTERRUPT();
    }
}

void do_switch (void)
{
    int         state;
    CpuCB_t     *cpu = getCpuCB();
    ProcCB_t    *pcb = getProcCB();

    if (cpu->intrOffNest != 0)
        kError(eSVC_Process, E_INTERRUPT);
    if (pcb->state == RUNNING)
        kError(eSVC_Process, E_PROCESS);
    if (intr_get())
        kError(eSVC_Process, E_INTERRUPT);

    state = cpu->intrOldState;
    switch_to(&pcb->context, &cpu->context);
    cpu->intrOldState = state;
}
void do_yield (void)
{
    kDISABLE_INTERRUPT();
    getProcCB()->state = READY;
    kENABLE_INTERRUPT();

    do_switch();
}
void do_suspend (void *obj)
{
    ProcCB_t *pcb = NULL;

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
void do_resume (void *obj)
{
    ProcCB_t        *pcb = NULL;
    list_entry_t    *ptr, *qtr;

    list_for_each_safe(ptr, qtr, &kPendList)
    {
        pcb = list_container_of(ptr, ProcCB_t, list);
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
int do_fork (void)
{
    int pid = -1;
    char *stack;
    ProcCB_t *newPcb;
    ProcCB_t *curPcb = getProcCB();

    stack = (char *)kalloc(curPcb->stackSize);
    if (stack == NULL)
        goto _exit_fork;

    /* 申请新的进程控制块 */
    newPcb = allocProcCB();
    if (newPcb == NULL)
    {
        kfree(stack);
        goto _exit_fork;
    }

    /* 完成两个进程间页表空间的复制 */
    // if (uvm_copy(newPcb->pageTab, curPcb->pageTab, curPcb->memSize, TRUE) < 0)
    // {
    //     kfree(stack);
    //     freeProcCB(newPcb);
    //     uvm_destroy(newPcb->pageTab);
    //     goto _exit_fork;
    // }

    kDISABLE_INTERRUPT();
    newPcb->stackSize = curPcb->stackSize;
    newPcb->memSize = curPcb->memSize;
    newPcb->stackAddr = (uint64)stack;
    newPcb->trapFrame->a0 = 0;
    newPcb->parent = curPcb;

    strcpy(newPcb->name, curPcb->name);
    memcpy(&newPcb->context, &curPcb->context, sizeof(Context_t));
    memcpy((void*)newPcb->stackAddr, (void*)curPcb->stackAddr, curPcb->stackSize);

    newPcb->context.sp = (uint64)(stack + (curPcb->context.sp - curPcb->stackAddr));
    wakeProcCB(newPcb);

    pid = newPcb->pid;
    kENABLE_INTERRUPT();

_exit_fork:
    return pid;
}
int do_wait (int *code)
{
    int pid = -1;
    ProcCB_t *childPcb, *curPcb;
    list_entry_t *ptr, *qtr;

    curPcb = getProcCB();
    while(1)
    {
        list_for_each_safe(ptr, qtr, &kRegistList)
        {
            childPcb = list_container_of(ptr, ProcCB_t, regist);

            /* 查找需要退出的子进程 */
            if ((childPcb->parent == curPcb) &&
                (childPcb->state == EXITING))
            {
		        /* 处理正在退出的子进程 */ 
                if (code != NULL)
                    *code = childPcb->exitState;

                pid = childPcb->pid;
                freeProcCB(childPcb);
                goto _exit_wait;
            }
            if (KillState(curPcb))
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
void do_exit (int state)
{
    ProcCB_t *curPcb;

    curPcb = getProcCB();
    if ((curPcb == kInitProcCB) ||
        (curPcb == kIdleProcCB))
        kError(eSVC_Process,E_DANGER);

    /* 确定当前进程是否有子进程 */ 
    freeChildProcCB(curPcb);

    curPcb->exitState = state;
    curPcb->state = EXITING;

    do_resume(curPcb->parent);
    do_switch();
}
int do_kill (int pid)
{
    ProcCB_t *pcb;

    pcb = findProcCB(pid);
    if ((pcb != NULL) && (pcb->state != EXITING))
    {
        kDISABLE_INTERRUPT();
        pcb->killState = 1;
        list_del_init(&pcb->regist);
        list_add(&kUnregistList, &pcb->regist);

        wakeProcCB(pcb);
        kENABLE_INTERRUPT();
        return 0;
    }
    return -1;
}
int do_sleep (int ms)
{
    timer_t *timer = NULL;
    ProcCB_t *pcb = NULL;

    if (ms != 0)
    {
        pcb = getProcCB();
        timer = timer_add(pcb, ms);

        do_switch();

        timer_del(timer);
    }

    return 0;
}
int KillState (ProcCB_t *pcb)
{
    return pcb->killState;
}
