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

/* 进程在用户空间(模式)中执行的函数 */ 
int main (int argc, char *argv[])
{
    // test_parameter(argc, argv);
    test_freeChildProcess();

    /* 直接退出测试 exit 接口 */
    return 0;
}


