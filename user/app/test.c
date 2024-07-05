/*
 * 用于测试 risc 架构用户空间的代码
 */


/* 进程在用户空间(模式)中执行的函数 */ 
void main (void)
{
    while(1)
    {
        asm volatile (
            "ecall"
        );
    }
}


