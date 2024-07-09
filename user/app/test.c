/*
 * 用于测试 risc 架构用户空间的代码
 */
#include "libc.h"

/* 进程在用户空间(模式)中执行的函数 */ 
int main (int argc, char *argv[])
{
    int i;

    printf("User space 'test' is running !\r\n");

    printf ("argc = %d\r\n", argc);
    for (i=0; i<8; i++)
    {
        if (argv[i] == NULL)
            break;
        printf ("argv[%d] = %s\r\n", i, argv[i]);
    }

    while(1)
    {
        sleep(1000);
    }

    /* 直接退出测试 exit 接口 */
    return 0;
}


