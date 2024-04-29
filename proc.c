
#include "defs.h"
#include "param.h"
#include "memlayout.h"



/**********************************************/
static uint64           kPidToken = 1;
static CpuCB_t          kCpusList[NCPU];
static list_entry_t     kProcList;
static list_entry_t     kReadyList;
static list_entry_t     kSleepList;

/**********************************************/
CpuCB_t *getCpuCB (void)
{
    return &kCpusList[getCpuID()];
}
ProcCB_t *getProcCB (void)
{
    return getCpuCB()->proc;
}
int allocPid (void)
{
    return (kPidToken++);
}
ProcCB_t *allocProcCB (void)
{
    return NULL;
}
int freeProcCB (ProcCB_t *obj)
{
    return 0;
}
void proc_init (void)
{
    list_init(&kProcList);
    list_init(&kReadyList);
    list_init(&kSleepList);
}

void scheduler (void)
{
    CpuCB_t         *cpu = getCpuCB();
    ProcCB_t        *pcb;
    list_entry_t    *ptr, *qtr;

    while(1)
    {
        intr_on();

        /* search all ready member of list */
        list_for_each_safe (ptr, qtr, &kReadyList)
        {
            pcb = list_container_of(ptr, ProcCB_t, list);
            if (pcb->state == READY)
            {
                kDISABLE_INTERRUPT();
                pcb->state = RUNNING;
                cpu->proc = pcb;
                kENABLE_INTERRUPT();

                switch_to(&cpu->context, &pcb->context);

                cpu->proc = NULL;
            }
        }
    }
}


void kswitch (void)
{
    int         state;
    CpuCB_t     *cpu = getCpuCB();
    ProcCB_t    *pcb = getProcCB();

    if (cpu->intrOffNest != 0)
        kError(errSVC_Process, errCode_InterruptNest);
    if (pcb->state == RUNNING)
        kError(errSVC_Process, errCode_ProcessState);
    if (intr_get())
        kError(errSVC_Process, errCode_InterruptState);

    state = cpu->intrOldState;
    switch_to(&pcb->context, &cpu->context);
    cpu->intrOldState = state;
}
void yield (void)
{
    kDISABLE_INTERRUPT();
    getProcCB()->state = READY;
    kENABLE_INTERRUPT();

    kswitch();
}
void sleep (void *obj)
{
    ProcCB_t *pcb = NULL;

    pcb = getProcCB();

    kDISABLE_INTERRUPT();
    pcb->sleepObj = obj;
    pcb->state = SLEEPING;
    list_del(&pcb->list);
    list_add(&kSleepList, &pcb->list);
    kENABLE_INTERRUPT();

    kswitch();

    pcb->sleepObj = NULL;
}
void wakeup (void *obj)
{
    ProcCB_t        *pcb = NULL;
    list_entry_t    *ptr, *qtr;

    list_for_each_safe(ptr, qtr, &kSleepList)
    {
        pcb = list_container_of(ptr, ProcCB_t, list);
        if (pcb->sleepObj == obj)
        {
            kDISABLE_INTERRUPT();
            pcb->state = READY;
            list_del(&pcb->list);
            list_add(&kReadyList, &pcb->list);
            kENABLE_INTERRUPT();
        }
    }
}
int create (void (*func)(void))
{
    #define STACK_SIZE  2*1024
    ProcCB_t *pcb = (ProcCB_t *)kalloc(sizeof(ProcCB_t));
    char *stack = (char *)kalloc(STACK_SIZE);

    memset(pcb, 0, sizeof(ProcCB_t));
    memset(stack, 0, STACK_SIZE);

    pcb->state = READY;
    pcb->pid = allocPid();
    pcb->context.ra = (uint64)func;
    pcb->context.sp = (uint64)(stack + STACK_SIZE);

    list_init(&pcb->list);
    list_add(&kProcList,  &pcb->list);
    list_add(&kReadyList, &pcb->list);
    return 0;
}
int fork (void)
{
    return 0;
}
void exit (int status)
{

}
int kill (int pid)
{
    return 0;
}
void setKillState (ProcCB_t *p)
{

}
int getKillState (ProcCB_t *p)
{
    return 0;
}
