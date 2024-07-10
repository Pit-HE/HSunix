
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

void exec (char *path, char *argv[])
{
    syscall(SYS_exec, path, argv);
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

int open (char *path, uint flags, uint mode)
{
    return syscall(SYS_open, path, flags, mode);
}

int close (int fd)
{
    return syscall(SYS_clone, fd);
}

int read (int fd, void *buf, int len)
{
    return syscall(SYS_read, fd, buf, len);
}

int write (int fd, void *buf, int len)
{
    return syscall(SYS_write, fd, buf, len);
}

int lseek (int fd, uint off, int whence)
{
    return syscall(SYS_lseek, fd, off, whence);
}

void fstat (void)
{

}

int fsync (int fd)
{
    return syscall(SYS_fsync);
}

void dup (void)
{
    syscall(SYS_dup);
}

void suspend (void *obj)
{
    syscall(SYS_suspend, obj);
}

void resume (void *obj)
{
    syscall(SYS_resume, obj);
}

int getdirent (int fd, void *buf, uint len)
{
    return syscall(SYS_getdirent, fd, buf, len);
}

int unlink (char *path)
{
    return syscall(SYS_unlink, path);
}

int chdir (char *path)
{
    return syscall(SYS_chdir, path);
}
