/*
 * 用于测试 risc 架构用户空间的代码
 */
#include "clib.h"

/* 进程在用户空间(模式)中执行的函数 */ 
void main (void)
{
    printf("User space code is running !\r\n");

    while(1)
    {
        sleep(1000);
    }
}


