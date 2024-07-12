
#include "types.h"
#include "syscall.h"


extern int syscall (int code, ...);


int open (char *path, uint flags, uint mode)
{
    return syscall(SYS_open, path, flags, mode);
}
int close (int fd)
{
    return syscall(SYS_close, fd);
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
int stat (char *path)
{
    return syscall(SYS_stat, path);
}
int fstatfs (int fd, void *buf)
{
    return syscall(SYS_fstatfs, fd, buf);
}
int fsync (int fd)
{
    return syscall(SYS_fsync);
}
void dup (void)
{
    syscall(SYS_dup);
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
int mount (char *fsname, char *path)
{
    return syscall(SYS_mount, fsname, path);
}
int umount (char *path)
{
    return syscall(SYS_umount, path);
}
int getcwd (char *buf, int len)
{
    return syscall(SYS_getcwd, buf, len);
}
int rename (char *oldname, char *newname)
{
    return syscall(SYS_rename, oldname, newname);
}
void exec (char *path, char *argv[])
{
    syscall(SYS_exec, path, argv);
}



