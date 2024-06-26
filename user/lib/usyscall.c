
#include "syscall.h"

extern int syscall (int code, ...);


void exit(int code)
{
    syscall(SYS_exit, code);
}

void fork (void)
{
    syscall(SYS_fork);
}

void wait (int *code)
{
    syscall(SYS_wait, code);
}

void exec (void)
{
    syscall(SYS_exec);
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

void pgdir (void)
{
    syscall(SYS_pgdir);
}

int gettime (void)
{
    return syscall(SYS_gettime);
}

void sleep (int ms)
{
    syscall(SYS_sleep, ms);
}

void open (void)
{
    syscall(SYS_open);
}

void close (void)
{
    syscall(SYS_clone);
}

void read (void)
{
    syscall(SYS_read);
}

void write (void)
{
    syscall(SYS_write);
}

void seek (void)
{
    syscall(SYS_seek);
}

void fstat (void)
{
    syscall(SYS_fstat);
}

void fsync (void)
{
    syscall(SYS_fsync);
}

void dup (void)
{
    syscall(SYS_dup);
}

