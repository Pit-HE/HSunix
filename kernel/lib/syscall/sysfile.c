
#include "syscall.h"
#include "fcntl.h"
#include "defs.h"
#include "file.h"
#include "fs.h"

uint64 sys_pgdir (void)
{
    return 0;
}
uint64 sys_open (void)
{
    char path[32];
    int flag, mode;

    arg_str(0, path, 32);
    arg_int(1, &flag);
    arg_int(2, &mode);
    return (uint64)vfs_open(path, flag, mode);
}
uint64 sys_close (void)
{
    int fd;

    arg_int(0, &fd);
    return vfs_close(fd);
}
uint64 sys_read (void)
{
    uint64 buf;
    int fd, len;

    arg_int (0, &fd);
    arg_addr(1, &buf);
    arg_int (2, &len);
    return vfs_read(fd, (void *)buf, len);
}
uint64 sys_write (void)
{
    uint64 buf;
    int fd, len;

    arg_int (0, &fd);
    arg_addr(1, &buf);
    arg_int (2, &len);
    return vfs_write(fd, (void *)buf, len);
}
uint64 sys_lseek (void)
{
    int fd, off, flag;

    arg_int (0, &fd);
    arg_int (1, &off);
    arg_int (2, &flag);
    return vfs_lseek(fd, off, flag);
}
uint64 sys_stat (void)
{
    char path[64];
    int fd, ret = -1;
    struct stat buf;

    arg_str(0, path, 64);

    fd = vfs_open(path, O_RDONLY, S_IRWXU);
    if (fd < 0)
        return -1;

    ret = vfs_stat(fd, &buf);
    vfs_close(fd);

    kprintf("    name: %s\r\n", buf.name);
    kprintf("    size: %d\r\n", buf.size);
    if (buf.type == VFS_DIR)
        kprintf("    type: dir\r\n");
    else
        kprintf("    type: file\r\n");
    kprintf("    fsname: %s\r\n", buf.fsname);

    return ret;
}
uint64 sys_fstatfs (void)
{
    int fd;
    uint64 buf;

    arg_int (0, &fd);
    arg_addr(1, &buf);
    return vfs_fstatfs(fd, (struct statfs *)buf);
}
uint64 sys_fsync (void)
{
    int fd;

    arg_int (0, &fd);
    return vfs_fsync(fd);
}
uint64 sys_dup (void)
{
    return 0;
}
uint64 sys_getdirent(void)
{
    uint64 buf;
    int fd, len;

    arg_int (0, &fd);
    arg_addr(1, &buf);
    arg_int (2, &len);
    return vfs_getdirent(fd, (void *)buf, len);
}
uint64 sys_unlink (void)
{
    char path[32];

    arg_str(0, path, 32);
    return vfs_unlink(path);
}
uint64 sys_chdir (void)
{
    int fd;
    char path[32];

    arg_str(0, path, 32);
    if (path == NULL)
        return -1;

    /* 确认该目录项存在 */
    fd = vfs_open(path, O_RDONLY | O_DIRECTORY, S_IRWXU);
    if (fd < 0)
        return -1;
    vfs_close(fd);

    return path_setcwd(path);
}
uint64 sys_mount (void)
{
    char name[16];
    char path[32];

    arg_str(0, name, 16);
    arg_str(1, path, 32);
    return vfs_mount(name, path, 
        O_RDWR | O_CREAT | O_DIRECTORY, NULL);
}
uint64 sys_umount (void)
{
    char path[32];

    arg_str(0, path, 32);
    return vfs_unmount(path);
}
uint64 sys_getcwd (void)
{
    int len;
    uint64 buf;

    arg_addr(0, &buf);
    arg_int (1, &len);
    kstrcpy((void *)buf, getProcCB()->cwd);
    return 0;
}
uint64 sys_rename (void)
{
    char oldname[16];
    char newname[16];

    arg_str(0, oldname, 16);
    arg_str(1, newname, 16);
    return vfs_rename(oldname, newname);
}
uint64 sys_exec (void)
{
    int i, ret = -1;
    char path[32], *argv[6];
    uint64 addr, uargv, tmp;
    struct ProcCB *pcb = getProcCB();

    arg_str (0, path, 32);
    arg_addr(1, &uargv);

    for (i=0; i<6; i++)
    {
        /* 获取用户空间存放数据的缓冲区地址 */
        tmp = uargv + (sizeof(uint64) * i);
        uvm_copyin(pcb->pgtab, (char *)&addr, tmp, sizeof(addr));

        /* 初始化 argv, 并确认是否有数据需要传递 */
        argv[i] = NULL;
        if (addr == 0)
            break;

        /* 申请存放数据的内核缓冲区 */
        argv[i] = kalloc(64);
        if (argv[i] == NULL)
            goto __err_sys_exec;

        /* 将用户层传递的数据写入内核数据缓冲区 */
        ret = uvm_copyin(pcb->pgtab, argv[i], addr, 64);
        if (ret < 0)
            goto __err_sys_exec;
    }
    /* 执行核心代码 */
    ret = do_exec(NULL, path, argv);

    /* 释放已经申请的内存空间 */
__err_sys_exec:
    for (i=0; i<6; i++)
    {
        if (argv[i] == NULL)
            break;
        kfree(argv[i]);
    }

    return ret;
}
