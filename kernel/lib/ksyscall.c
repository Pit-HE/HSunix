/*
 * 当前文件主要记录内核对用户层提供的系统调用接口
 */
#include "ksyscall.h"
#include "defs.h"
#include "proc.h"


static uint64 sys_exit (int arg[])
{
    do_exit(arg[0]);
    return 0;
}
static uint64 sys_fork (int arg[])
{
    return do_fork();
}
static uint64 sys_wait (int arg[])
{
    return do_wait(&arg[0]);
}
static uint64 sys_exec (int arg[])
{
    return 0;
}
static uint64 sys_yield (int arg[])
{
    do_yield();
    return 0;
}
static uint64 sys_kill (int arg[])
{
    return do_kill(arg[0]);
}
static uint64 sys_getpid (int arg[])
{
    return getProcCB()->pid;
}
static uint64 sys_putc (int arg[])
{
    console_wChar(NULL, (char)arg[0]);
    return 0;
}
static uint64 sys_pgdir (int arg[])
{
    return 0;
}
static uint64 sys_gettime (int arg[])
{
    extern uint64 Systicks;
    return Systicks;
}
static uint64 sys_sleep (int arg[])
{
    return do_sleep(arg[0]);
}
static uint64 sys_open (int arg[])
{
    return 0;
}
static uint64 sys_close (int arg[])
{
    return 0;
}
static uint64 sys_read (int arg[])
{
    return 0;
}
static uint64 sys_write (int arg[])
{
    return 0;
}
static uint64 sys_seek (int arg[])
{
    return 0;
}
static uint64 sys_fstat (int arg[])
{
    return 0;
}
static uint64 sys_fsync (int arg[])
{
    return 0;
}
static uint64 sys_dup (int arg[])
{
    return 0;
}

static uint64 (*syscall_entry[])(int arg[]) =
{
    [SYS_exit]      sys_exit,
    [SYS_fork]      sys_fork,
    [SYS_wait]      sys_wait,
    [SYS_exec]      sys_exec,
    [SYS_yield]     sys_yield,
    [SYS_kill]      sys_kill,
    [SYS_getpid]    sys_getpid,
    [SYS_putc]      sys_putc,
    [SYS_pgdir]     sys_pgdir,
    [SYS_gettime]   sys_gettime,
    [SYS_sleep]     sys_sleep,
    [SYS_open]      sys_open,
    [SYS_close]     sys_close,
    [SYS_read]      sys_read,
    [SYS_write]     sys_write,
    [SYS_seek]      sys_seek,
    [SYS_fstat]     sys_fstat,
    [SYS_fsync]     sys_fsync,
    [SYS_dup]       sys_dup,
};
#define SYSCALL_NUM (sizeof(syscall_entry)/sizeof(syscall_entry[0]))



void do_syscall (void)
{
    uint code;
    int args[7];
    struct ProcCB *pcb = getProcCB();

    code = pcb->trapFrame->a7;

    if ((code < SYSCALL_NUM) &&
        (syscall_entry[code] != NULL))
    {
        args[0] = pcb->trapFrame->a0;
        args[1] = pcb->trapFrame->a1;
        args[2] = pcb->trapFrame->a2;
        args[3] = pcb->trapFrame->a3;
        args[4] = pcb->trapFrame->a4;
        args[5] = pcb->trapFrame->a5;
        args[5] = pcb->trapFrame->a6;

        pcb->trapFrame->a0 = syscall_entry[code](args);
    }
    else
    {
        kprintf ("This system call is invalid !\r\n");
        pcb->trapFrame->a0 = -1;
    }
}
