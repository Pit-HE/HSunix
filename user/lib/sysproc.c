
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

int wait (int *code)
{
    return syscall(SYS_wait, code);
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

int brk (void)
{
    return syscall(SYS_brk);
}

int msgget(int key, int msgflg)
{
    return syscall(SYS_msgget, key, msgflg);
}

int msgsnd(int msqid, void *msgp, uint msgsz, int msgflg)
{
    return syscall(SYS_msgsnd, msqid, msgp, msgsz, msgflg);
}

int msgrcv(int msqid, void *msgp, uint msgsz,
        int msgtyp, int msgflg)
{
    return syscall(SYS_msgrcv, msqid, msgp, msgsz,
        msgtyp, msgflg);
}

int msgctl(int msqid, int cmd, void *uptr)
{
    return syscall(SYS_msgctl, msqid, cmd, uptr);
}
