/*
 * 用于测试 risc 架构用户空间的代码
 */
#include "clib.h"


/* 进程在用户空间(模式)中执行的函数 */ 
void user_first (void)
{
    while(1)
    {
        sleep(1000);
    }
}

