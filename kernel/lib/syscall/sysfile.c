
#include "syscall.h"
#include "defs.h"
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
uint64 sys_fstat (void)
{
    return 0;
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
