/*
 * init 进程相关的源码
 */

#include "defs.h"
#include "fcntl.h"


void init_main (void)
{
    if (-1 == vfs_pcbInit(getProcCB(), "/"))
        kError(eSVC_Process, E_STATUS);

    while (1)
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
        do_scheduler();
        do_defuncter();
    }
}





/* 在文件系统中创建默认的文件树结构 */
void default_fsdir (void)
{
    mkdir("/fs", S_IRWXU);
    
    mkdir("/usr", S_IRWXU);
    mkdir("/usr/tmp1", S_IRWXU);
    mkdir("/usr/tmp2", S_IRWXU);

    mkdir("/bin", S_IRWXU);
    mkdir("/bin/bin", S_IRWXU);
    mkdir("/bin/bin/stop", S_IRWXU);
    mkfile("/bin/a.a", O_CREAT|O_RDWR, S_IRWXU);
    mkfile("/bin/b.b", O_CREAT|O_RDWR, S_IRWXU);

    mkdir("/home", S_IRWXU);
    mkfile("/home/abc.o", O_CREAT|O_RDWR, S_IRWXU);
    mkfile("/home/123.o", O_CREAT|O_RDWR, S_IRWXU);
    mkdir("/home/study", S_IRWXU);
    mkdir("/home/study/tmp1", S_IRWXU);
    mkdir("/home/study/tmp2", S_IRWXU);
    mkdir("/home/book", S_IRWXU);
    mkdir("/home/book/usr", S_IRWXU);
    mkdir("/home/book/sys", S_IRWXU);
    mkfile("/home/book/a.a", O_CREAT|O_RDWR, S_IRWXU);
    mkfile("/home/book/b.b", O_CREAT|O_RDWR, S_IRWXU);

    mkfile("/a.a", O_CREAT|O_RDWR, S_IRWXU);
    mkfile("/b.b", O_CREAT|O_RDWR, S_IRWXU);
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
    default_fsdir();

    while(1)
    {
        do_sleep(100);
    }
 #endif
}
