
#include "types.h"
#include "syscall.h"


extern int syscall (int code, ...);


void exit(int code)
{
    syscall(SYS_exit, code);
}

int fork (void)
{
    return syscall(SYS_fork);
}

void wait (int *code)
{
    syscall(SYS_wait, code);
}

void yield (void)
{
    syscall (SYS_yield);
}

void kill (int pid)
{
    syscall(SYS_kill, pid);
}

int getpid (void)
{
    return syscall(SYS_getpid);
}

void putc (int ch)
{
    syscall(SYS_putc, ch);
}

int getc (void)
{
    return syscall(SYS_getc);
}

void suspend (void *obj)
{
    syscall(SYS_suspend, obj);
}

void resume (void *obj)
{
    syscall(SYS_resume, obj);
}

int gettime (void)
{
    return syscall(SYS_gettime);
}

void sleep (int ms)
{
    syscall(SYS_sleep, ms);
}





