
#include "syscall.h"
#include "defs.h"
#include "proc.h"

uint64 sys_exit (void)
{
    int num;

    arg_int(0, &num);
    do_exit(num);

    return 0;
}
uint64 sys_fork (void)
{
    return do_fork();
}
uint64 sys_wait (void)
{
    int num;

    do_wait(&num);
    return num;
}
uint64 sys_yield (void)
{
    do_yield();
    return 0;
}
uint64 sys_kill (void)
{
    int num;

    arg_int(0, &num);
    return do_kill(num);
}
uint64 sys_getpid (void)
{
    return getProcCB()->pid;
}
uint64 sys_putc (void)
{
    int num;

    arg_int(0, &num);
    console_wChar(NULL, num);
    return 0;
}
uint64 sys_getc (void)
{
    return (uint64)console_rChar();
}
uint64 sys_suspend (void)
{
    uint64 num;

    arg_addr(0, &num);
    do_suspend((void *)num);
    return 0;
}
uint64 sys_resume (void)
{
    uint64 num;

    arg_addr(0, &num);
    do_resume((void *)num);
    return 0;
}
uint64 sys_gettime (void)
{
    extern uint64 Systicks;
    return Systicks;
}
uint64 sys_sleep (void)
{
    int num;

    arg_int(0, &num);
    return do_sleep(num);
}