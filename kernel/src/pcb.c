/*
 * 存放处理进程控制块相关功能的接口
 */
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "proc.h"

/* 作为指针，指向 trampoline.S 里的代码 */
extern char trampoline[];

extern uint64       kPidToken;
extern struct CpuCB        kCpusList[NCPU];
extern ListEntry_t  kUnregistList;
extern ListEntry_t  kRegistList;
extern ListEntry_t  kReadyList;
extern ListEntry_t  kPendList;
extern struct ProcCB      *kInitProcCB;
extern struct ProcCB      *kIdleProcCB;


struct ProcCB *getProcCB (void)
{
    return getCpuCB()->proc;
}

/* 打印当前非空闲进程的信息 */
void pcb_dump (void)
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
    ListEntry_t *ptr;
    struct ProcCB *pcb;
    char *state;

    kprintf("\n");

    /* 遍历进程控制块数组 */
    list_for_each(ptr, &kRegistList)
    {
        pcb = list_container_of(ptr, struct ProcCB, regist);

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


/* 申请并初始化一个进程控制块的内存空间 */
struct ProcCB *pcb_alloc (void)
{
    struct ProcCB *pcb = NULL;

    /* 申请任务控制块的内存空间 */
    pcb = (struct ProcCB *)kalloc(sizeof(struct ProcCB));
    if (pcb == NULL)
        return NULL;
    
    /* 给进程申请存储 trap 上下文的虚拟地址空间 */
    pcb->trapFrame = kallocPhyPage();
    if (pcb->trapFrame == NULL)
    {
        kfree (pcb);
        return NULL;
    }

    /* 申请进程的虚拟页表并初始化 */
    pcb->pageTab = proc_alloc_pgtab(pcb);
    if (pcb->pageTab == NULL)
    {
        kfreePhyPage(pcb->trapFrame);
        kfree (pcb);
        return NULL;
    }

    /* 清空进程切换时的内核上下文 */
    kmemset(&pcb->context, 0, sizeof(struct Context));

    /* 设置进程的信息 */
    pcb->pid = proc_applypid();
    pcb->state = USED;

    /* 初始化进程的链表信息 */
    list_init(&pcb->list);
    list_init(&pcb->regist);

    /* 将进程添加到内核的注册链表中 */
    list_add (&kRegistList, &pcb->regist);

    return pcb;
}

/* 释放已经存在的进程控制块 */
int pcb_free (struct ProcCB *pcb)
{
    if (pcb == NULL)
        return -1;

    kDISABLE_INTERRUPT();
    /* 移除进程所挂载的链表 */
    list_del_init(&pcb->regist);
    list_del_init(&pcb->list);

    /* 是否进程页表内所占用的内存空间 */
    proc_free_pgtab(pcb->pageTab, pcb->memSize);
    pcb->pageTab = NULL;

    /* 释放进程控制块的资源 */
    kfree(pcb);

    /* 释放进程的文件描述符数组 */
    fdTab_free(pcb);

    kENABLE_INTERRUPT();

    return 1;
}

/* 寻找指定 id 的进程控制块 */
struct ProcCB *pcb_lookup (int pid)
{
    struct ProcCB *pcb = NULL;
    ListEntry_t *ptr = NULL;

    if ((kPidToken < pid) && (pid < 0))
        goto exit_findProcCB;

    /* 遍历链表，查找之地内的 ID */
    list_for_each(ptr, &kRegistList)
    {
        pcb = list_container_of(ptr, struct ProcCB, regist);
        if (pcb->pid == pid)
            goto exit_findProcCB;
    }
 exit_findProcCB:
    return pcb;
}


/* 为进程申请虚拟页表并初始化默认的虚拟地址空间 
 * ( 与 proc_free_pgtab 成对使用 ) 
 */
pgtab_t *proc_alloc_pgtab (struct ProcCB *pcb)
{
    pgtab_t *pgtab = NULL;

    /* 创建新的进程页表 */
    pgtab = uvm_create();
    if (pgtab == NULL)
        return NULL;
    
    /* 给进程申请存放进出内核空间代码的地址 */
    if (0 > kvm_map(pgtab, TRAMPOLINE, (uint64)trampoline, PGSIZE, PTE_R|PTE_X))
    {
        uvm_destroy(pgtab, 0);
        return NULL;
    }

    /* 将进程存储 trap 上下文的物理地址映射到指定的虚拟地址 */
    if (0 > kvm_map(pgtab, TRAPFRAME, (uint64)pcb->trapFrame, PGSIZE, PTE_R|PTE_W))
    {
        uvm_unmap(pgtab, TRAMPOLINE, 1, TRUE);
        uvm_destroy(pgtab, 0);
        return NULL;
    }

    return pgtab;
}

/* 释放进程虚拟页表占用的所有物理内存空间 
 * ( 与 proc_alloc_pgtab 成对使用 ) 
 */
int proc_free_pgtab (pgtab_t *pgtab, uint64 size)
{
    /* 处理特殊页已经映射的物理内存 (不做释放) */
    uvm_unmap(pgtab, TRAPFRAME , 1, FALSE);
    uvm_unmap(pgtab, TRAMPOLINE, 1, FALSE);

    /* 释放进程用户空间与页表本身占用的物理内存 */
    uvm_destroy(pgtab, size);
    return 0;
}

