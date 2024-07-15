/*
 * 用于测试 risc 架构用户空间的代码
 */
#include "libc.h"


/* 测试进程在用户空间与内核空间之间的参数传递 */
void test_parameter (int argc, char *argv[])
{
    int i;

    printf("Test process: argc = %d\r\n",argc);
    printf ("argc = %d\r\n", argc);
    for (i=0; i<8; i++)
    {
        if (argv[i] == NULL)
            break;
        printf ("argv[%d] = %s\r\n", i, argv[i]);
    }
}

/* 测试 init 进程处理子进程释放的功能 */
void test_freeChildProcess (void)
{
    int pid = fork();
    char *argv[1] = {NULL};

    if (pid == 0)
        exec ("/bin/pwd", argv);
}

/* 测试管道在两个进程间数据传递的功能 */
void test_pipeDataTransfer (void)
{
    int fd[2], i, pid;
    char rbuf[32], wbuf[32];

    pipe(fd);
    pid = fork();

    if (pid == 0)
    {
        /* 子进程等待数据写入管道后读取 */
        read (fd[0], rbuf, 32);

        printf ("rbuf = ");
        for (i=0; i<32; i++)
            printf ("%d ", rbuf[i]);
        printf ("\r\n");
        exit(0);
    }
    else
    {
        /* 父进程将数据写入管道，等待子进程释放 */
        for (i=0; i<32; i++)
            wbuf[i] = i;
        write(fd[1], wbuf, 32);

        printf ("fork pid = %d\r\n", pid);
        pid = wait(0);
        printf ("wait pid = %d\r\n", pid);
    }

    printf ("Print only once !\r\n");
    close(fd[0]);
    close(fd[1]);
}

/* 进程在用户空间(模式)中执行的函数 */ 
int main (int argc, char *argv[])
{
    // test_parameter(argc, argv);
    // test_freeChildProcess();
    test_pipeDataTransfer();

    /* 直接退出测试 exit 接口 */
    return 0;
}


