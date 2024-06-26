/*
 * 用于测试 risc 架构用户空间的代码
 */
#include "clib.h"


/* 进程在用户空间(模式)中执行的函数 */ 
void user_first (void)
{
    while(1)
    {
        /* 通过系统接口打印用户空间的内容 */ 
        uprintf ("userspace: pid = %d\r\n",getpid());
        uprintf ("userspace: time = %d\r\n", gettime());

        sleep(1000);
    }
}

