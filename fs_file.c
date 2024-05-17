/*
 * 文件系统内操作具体文件描述符所存储信息的模块
 */
#include "defs.h"
#include "file.h"

/* 提取文件描述符中的 inode 信息，打开具体的对象
 *
 * path:
 *      1、传入的字符串以斜杆('/')、点('.' 或 '..') 开头的则为文件系统对象
 *      2、传入的是纯粹字符串的则是非文件系统对象
 */
int file_open (struct File *f, const char *path, uint flags)
{
    struct Inode *inode;

    inode = inode_alloc();
    if (inode == NULL)
    {
        kErr_printf("fail: file_open alloc inode !\r\n");
        return -1;
    }




    return 0;
}

int file_close (struct File *f)
{
    return 0;
}

int file_read (struct File *f, void *buf, uint len)
{
    return 0;
}

int file_write (struct File *f, void *buf, uint len)
{
    return 0;
}

int file_flush (struct File *f)
{
    return 0;
}

