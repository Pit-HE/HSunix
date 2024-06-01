/*
 * init 进程相关的源码
 */

#include "defs.h"
#include "fcntl.h"


void init_main (void)
{
    if (-1 == vfs_pcbInit(getProcCB(), "/"))
        kError(eSVC_Process, E_STATUS);

    kprintf("admin:~/ ");

    while(1)
    {
        cli_main();
    }
}

void idle_main (void)
{
    if (-1 == vfs_pcbInit(getProcCB(), "/"))
        kError(eSVC_Process, E_STATUS);

    while(1)
    {
        scheduler();
        defuncter();
    }
}


void test_main (void)
{
 #if 0
    static int testCnt = 0;
    static int testPid = 0;
    ProcCB *pcb = getProcCB();

    if (testCnt <= 2)
    {
        testCnt++;
        do_fork();
    }

    if (testCnt == 3)
    {
        testPid = pcb->pid;
        while(1)
        {
            kprintf ("pid = %d\r\n", pcb->pid);
            do_suspend((void*)pcb);
        }
    }
    else if(testCnt == 2)
    {
        while(1)
        {
            if (testCnt == 5)
                do_kill(testPid);
            if (testCnt >= 10)
                do_exit(88);

            kprintf ("pid = %d, cnt = %d\r\n", pcb->pid, testCnt++);
            do_sleep(100);
        }
    }
    else if(testCnt == 1)
    {
        int childCode;
        int childPid;

        kprintf ("pid = %d parent process wait()\r\n", pcb->pid);
        while(1)
        {
            childPid = do_wait(&childCode);
            kprintf ("childPid = %d, childCode = %d\r\n", childPid, childCode);
        }
    }
 #elif 1
    if (-1 == vfs_pcbInit(getProcCB(), "/"))
        kError(eSVC_Process, E_STATUS);

    /* 调用测试用例执行测试 */
    extern void proc_selfInspection (void);
    proc_selfInspection();

    while(1)
    {
        do_sleep(100);
    }
 #endif
}
