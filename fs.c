/*
 * 文件系统对外的操作接口
 */
#include "defs.h"
#include "file.h"
#include "kerror.h"


/* 文件系统对外接口：打开指定路径的文件 */
int fs_open (const char *path, int flags)
{
    int fd;
    struct File *file = NULL;

    fd = fd_alloc();
    if (fd < 0)
    {
        kErr_printf("fail: fs_open alloc fd !\r\n");
        return -1;
    }

    file = fd_get(fd);
    if (file == NULL)
    {
        kErr_printf("fail: fs_open get fd !\r\n");
        return -1;
    }

    if (0 > file_open(file, (char *)path, flags))
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
    struct File *file;

    file = fd_get(fd);
    if (file < 0)
    {
        kErr_printf("fail: fs_close get fd !\r\n");
        return -1;
    }

    ret = file_close(file);
    if (0 > ret)
    {
        kErr_printf("fail: fs_close close file !\r\n");
        return -1;
    }

    fd_free(fd);

    return ret;
}

/* 文件系统对外接口：将数据写入指定文件中 */
int fs_write (int fd, void *buf, int len)
{
    int ret;
    struct File *file;

    file = fd_get(fd);
    if (file < 0)
    {
        kErr_printf("fail: fs_write get fd !\r\n");
        return -1;
    }

    ret = file_write(file, buf, len);
    if (0 > ret)
    {
        kErr_printf("fail: fs_write write file !\r\n");
        return -1;
    }

    return ret;
}

/* 文件系统对外接口：从指定文件中读取指定长度的数据 */
int fs_read (int fd, void *buf, int len)
{
    int ret;
    struct File *file;

    file = fd_get(fd);
    if (file < 0)
    {
        kErr_printf("fail: fs_read get fd !\r\n");
        return -1;
    }

    ret = file_read(file, buf, len);
    if (ret < 0)
    {
        kErr_printf("fail: fs_read read file !\r\n");
        return -1;
    }

    return ret;
}


