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

/* 测试线程库的基本功能是否正常 */
void *thread (void *param)
{
    int i = 0;
    while(1)
    {
        printf ("%s cnt = %d\r\n", 
            (char *)param, i);
        pthread_sleep(5000);
        if (++i >= 3)
            break;
    }
    return NULL;
}
void test_threadscheduler (void)
{
    int pid; 
    pthread_t tid1, tid2;
    static char arg1[] = "thread1";
    static char arg2[] = "thread2";

    pid = fork();
    if (0 == pid)
    {
        pthread_create(&tid1, NULL, thread, arg1);
        pthread_create(&tid2, NULL, thread, arg2);
        pthread_join(tid1, NULL);
        pthread_join(tid2, NULL);
        exit(66);
    }
}

/* 测试消息队列在两个进程间传递数据 */
void test_msgDataTransfer (void)
{
    msg_info info;
    int pid, mid, i = 0;
    
    pid = fork();
    if (pid == 0)
    {
        mid = msgget(1234, IPC_CREAT | 0x666);

        info.mtext = malloc(32);
        for (i=0; i<6; i++)
        {
            info.type = i;
            memset(info.mtext, 0, 32);
            switch(i)
            {
                case 0:strcpy(info.mtext, "1111111");
                    break;
                case 1:strcpy(info.mtext, "2222222");
                    break;
                case 2:strcpy(info.mtext, "3333333");
                    break;
                case 3:strcpy(info.mtext, "4444444");
                    break;
                case 4:strcpy(info.mtext, "5555555");
                    break;
                default:strcpy(info.mtext, "6666666");
                    break;
            }
            msgsnd(mid, (void *)&info, 32, 0);
        }
    }
    else
    {
        mid = msgget(1234, IPC_CREAT | 0x666);

        info.mtext = malloc(32);
        for (i=0; i<6; i++)
        {
            if (msgrcv(mid, (void *)&info, 32, i, 0) > 0)
            {
                printf ("msgrcv: type=%ld, text=%s\r\n",
                    info.type, (char *)info.mtext);
            }
        }
    }
}

/* 进程在用户空间(模式)中执行的函数 */ 
int main (int argc, char *argv[])
{
    /* 创建子进程去执行对应的测试用例 */
    if (fork() > 0)
        return 0;

    if (strncmp(argv[1], "param", 5) == 0)
    {
        printf ("test: test_parameter\r\n");
        test_parameter(argc, argv);
    }
    else if (strncmp(argv[1], "init", 4) == 0)
    {
        printf ("test: test_freeChildProcess\r\n");
        test_freeChildProcess();
    }
    else if (strncmp(argv[1], "pipe", 5) == 0)
    {
        printf ("test: test_pipeDataTransfer\r\n");
        test_pipeDataTransfer();
    }
    else if (strncmp(argv[1], "thread", 6) == 0)
    {
        printf ("test: test_threadscheduler\r\n");
        test_threadscheduler();
    }
    else if (strncmp(argv[1], "msg", 3) == 0)
    {
        printf ("test: test_msgDataTransfer\r\n");
        test_msgDataTransfer();
    }

    return 0;
}


