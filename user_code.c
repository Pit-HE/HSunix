
#include "defs.h"
#include "user_lib.h"




/* 进程在用户空间(模式)中执行的函数 */ 
uint64 userCnt = 0;
void user_processEntry (void)
{
    while(1)
    {
        userCnt++;

        /* 通过系统接口打印用户空间的内容 */ 
        printf ("userspace: pid = %d\r\n",getpid());
        printf ("userspace: cnt = %d\r\n", userCnt);
        printf ("userspace: time = %d\r\n", gettime());

        sleep(1000);
    }
}

