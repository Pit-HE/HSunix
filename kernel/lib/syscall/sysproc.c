
#include "syspriv.h"
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
    uint64 code;

    arg_addr(0, &code);
    code = kvm_phyaddr(getProcCB()->pgtab, code);

    return do_wait((int *)code);
}
uint64 sys_yield (void)
{
    do_yield();
    return 0;
}
uint64 sys_kill (void)
{
    int pid;

    arg_int(0, &pid);
    return do_kill(pid);
}
uint64 sys_getpid (void)
{
    return getProcCB()->pid;
}
uint64 sys_putc (void)
{
    int ch;

    arg_int(0, &ch);
    console_wChar(NULL, ch);
    return 0;
}
uint64 sys_getc (void)
{
    return console_rChar();
}
uint64 sys_suspend (void)
{
    uint64 obj;

    arg_addr(0, &obj);
    do_suspend((void *)obj);
    return 0;
}
uint64 sys_resume (void)
{
    uint64 obj;

    arg_addr(0, &obj);
    do_resume((void *)obj);
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