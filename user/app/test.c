/*
 * 用于测试 risc 架构用户空间的代码
 */
#include "clib.h"

/* 进程在用户空间(模式)中执行的函数 */ 
int main (int argc, char *argv[])
{
    printf("User space code is running !\r\n");

    /* 直接退出测试 exit 接口 */
    return 0;
}


