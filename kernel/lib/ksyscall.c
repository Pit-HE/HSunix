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
static uint64 sys_exec (int argv[])
{
    return do_exec(NULL, (char *)argv[0], (char **)argv[1]);
}
static uint64 sys_yield (int argv[])
{
    do_yield();
    return 0;
}
static uint64 sys_kill (int argv[])
{
    return do_kill(argv[0]);
}
static uint64 sys_getpid (int argv[])
{
    return getProcCB()->pid;
}
static uint64 sys_putc (int argv[])
{
    console_wChar(NULL, (char)argv[0]);
    return 0;
}
static uint64 sys_getcc (int argv[])
{
    return (uint64)console_rChar();
}
static uint64 sys_suspend (int argv[])
{
    do_suspend((void*)argv);
    return 0;
}
static uint64 sys_resume (int argv[])
{
    do_resume((void*)argv);
    return 0;
}
static uint64 sys_pgdir (int argv[])
{
    return 0;
}
static uint64 sys_gettime (int argv[])
{
    extern uint64 Systicks;
    return Systicks;
}
static uint64 sys_sleep (int argv[])
{
    return do_sleep(argv[0]);
}
static uint64 sys_open (int argv[])
{
    return vfs_open((char *)argv[0], argv[1], argv[2]);
}
static uint64 sys_close (int argv[])
{
    return vfs_close(argv[0]);
}
static uint64 sys_read (int argv[])
{
    return vfs_read(argv[0], (void *)argv[1], argv[2]);
}
static uint64 sys_write (int argv[])
{
    return vfs_write(argv[0], (void *)argv[1], argv[2]);
}
static uint64 sys_lseek (int argv[])
{
    return vfs_lseek(argv[0], argv[1], argv[2]);
}
static uint64 sys_fstat (int arg[])
{
    return 0;
}
static uint64 sys_fsync (int argv[])
{
    return vfs_fsync(argv[0]);
}
static uint64 sys_dup (int argv[])
{
    return 0;
}

static uint64 (*syscall_entry[])(int argv[]) =
{
    [SYS_exit]      sys_exit,
    [SYS_fork]      sys_fork,
    [SYS_wait]      sys_wait,
    [SYS_exec]      sys_exec,
    [SYS_yield]     sys_yield,
    [SYS_kill]      sys_kill,
    [SYS_getpid]    sys_getpid,
    [SYS_putc]      sys_putc,
    [SYS_getc]      sys_getcc,
    [SYS_pgdir]     sys_pgdir,
    [SYS_gettime]   sys_gettime,
    [SYS_sleep]     sys_sleep,
    [SYS_suspend]   sys_suspend,
    [SYS_resume]    sys_resume,
    [SYS_open]      sys_open,
    [SYS_close]     sys_close,
    [SYS_read]      sys_read,
    [SYS_write]     sys_write,
    [SYS_lseek]     sys_lseek,
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
        args[6] = pcb->trapFrame->a6;

        pcb->trapFrame->a0 = syscall_entry[code](args);
    }
    else
    {
        kprintf ("This system call is invalid !\r\n");
        pcb->trapFrame->a0 = -1;
    }
}
