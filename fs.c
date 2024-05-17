/*
 * 提供文件系统中，对外的操作接口
 */
#include "defs.h"
#include "file.h"
#include "kerror.h"


/* 文件系统对外接口：打开指定路径的文件 */
int fs_open (const char *file, int flags)
{
    int fd;
    struct File *f = NULL;

    fd = fd_alloc();
    if (fd < 0)
    {
        kErr_printf("fail: fs_open alloc fd !\r\n");
        return -1;
    }

    f = fd_get(fd);
    if (f == NULL)
    {
        kErr_printf("fail: fs_open get fd !\r\n");
        return -1;
    }

    if (0 > file_open(f, file, flags))
    {
        kErr_printf("fail: fs_open open file !\r\n");
        return -1;
    }

    return fd;
}

/* 文件系统对外接口：关闭已打开的文件 */
int fs_close (int fd)
{
    int ret;
    struct File *f;

    f = fd_get(fd);
    if (f < 0)
    {
        kErr_printf("fail: fs_close get fd !\r\n");
        return -1;
    }

    ret = file_close(f);
    if (0 > ret)
    {
        kErr_printf("fail: fs_close close file !\r\n");
        return -1;
    }

    fd_put(f);

    return ret;
}

/* 文件系统对外接口：将数据写入指定文件中 */
int fs_write (int fd, void *buf, uint len)
{
    int ret;
    struct File *f;

    f = fd_get(fd);
    if (f < 0)
    {
        kErr_printf("fail: fs_write get fd !\r\n");
        return -1;
    }

    ret = file_write(f, buf, len);
    if (0 > ret)
    {
        kErr_printf("fail: fs_write write file !\r\n");
        return -1;
    }

    return ret;
}

/* 文件系统对外接口：从指定文件中读取指定长度的数据 */
int fs_read (int fd, void *buf, uint len)
{
    int ret;
    struct File *f;

    f = fd_get(fd);
    if (f < 0)
    {
        kErr_printf("fail: fs_read get fd !\r\n");
        return -1;
    }

    ret = file_read(f, buf, len);
    if (ret < 0)
    {
        kErr_printf("fail: fs_read read file !\r\n");
        return -1;
    }

    return ret;
}


