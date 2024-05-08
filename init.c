
#include "defs.h"


void init_main (void)
{
    kprintf("sh: ");

    while(1)
    {
        cli_main();
    }
}

void idle_main (void)
{
    while(1)
    {
        scheduler();
        defuncter();
    }
}

int testCnt = 0;
int testPid = 0;
void test_main (void)
{
    ProcCB_t *pcb = getProcCB();

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
                do_exit(999);

            kprintf ("pid = %d, cnt = %d\r\n", pcb->pid, testCnt++);
            do_sleep(1000);
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
}
