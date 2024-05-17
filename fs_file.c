/*
 * 管理并操作文件描述符对象
 */
#include "defs.h"
#include "file.h"

/* 申请一个文件描述符的内存空间
 *
 * 返回值：NULL 为失败
 */
struct File *file_alloc (void)
{
    struct File *file = NULL;

    file = (struct File *)kalloc(sizeof(struct File));
    if (file == NULL)
        return file;
    kmemset(file, 0, sizeof(struct File));

    file->magic = FILE_MAGIC;

    return file;
}

/* 释放已申请的文件描述符 */
void file_free (struct File *file)
{
    if (file == NULL)
        return;
    if (file->magic != FILE_MAGIC)
        return;
    if (file->ref != 0)
        return;

    kmemset(file, 0, sizeof(struct File));
    kfree(file);
}


/* 打开文件系统中要操作的对象，将其信息存入文件描述符
 *
 *
 * file: 要操作的文件描述符对象
 *
 * path:
 *      1、传入的字符串以斜杆('/')、点('.' 或 '..') 开头的则为文件系统对象
 *      2、传入的是纯粹字符串的则是非文件系统对象
 *
 * flags: 文件操作的类型
 *
 *
 * 返回值：-1表示失败
 */
int file_open (struct File *file, char *path, uint flags)
{
    int ret = 1;
    struct Device *dev;
    struct Inode *inode;

    if ((file == NULL) || (path == NULL))
        return -1;
    if (file->magic != FILE_MAGIC)
        return -1;

    /* 创建存放关键信息 inode */
    inode = inode_alloc();
    if (inode == NULL)
        return -1;

    /* 判断要操作的对象类型 */
    if ((*path == '/') || (*path == '.'))
    {
        /* 操作的是文件系统对象 */
        if (0 > path_parser(path, inode))
            return -1;
    }
    else
    {
        /* 操作的是设备、管道等对象 */
        dev = dev_get(path);
        if (dev == NULL)
            return -1;

        /* 记录设备的信息 */
        inode->dev  = dev;
        inode->type = I_DEVICE;
        inode->i_ops  = &dev->opt;
    }
    ret = inode_open(inode);

    /* 初始化新打开的文件描述符 */
    file->inode  = inode;
    file->ref   += 1;
    file->offset = 0;
    file->flags  = flags;

    return ret;
}

/* 关闭已打开的文件系统对象，注销已打开的文件描述符
 *
 * 返回值：-1表示失败
 */
int file_close (struct File *file)
{
    int ret;
    struct Inode *inode;

    if ((file == NULL) || (file->magic != FILE_MAGIC))
        return -1;
    if (file->ref <= 0)
        return -1;

    file->ref -= 1;
    inode = file->inode;

    ret = inode_close(inode);

    /* 释放 open 中申请的 inode */
    inode_free(inode);

    return ret;
}

/* 读取文件数据
 *
 * 返回值：-1表示失败，其他值表示实际读取的长度
 */
int file_read (struct File *file, void *buf, uint len)
{
    if ((file == NULL) || (buf == NULL))
        return -1;
    if (file->magic != FILE_MAGIC)
        return -1;
    if (file->ref <= 0)
        return -1;

    return inode_read(file->inode, buf, len);
}

/* 将数据写入文件
 *
 * 返回值：-1表示失败，其他值表示实际写入的长度
 */
int file_write (struct File *file, void *buf, uint len)
{
    if ((file == NULL) || (buf == NULL))
        return -1;
    if (file->magic != FILE_MAGIC)
        return -1;
    if (file->ref <= 0)
        return -1;

    return inode_write(file->inode, buf, len);
}

/* 将文件在内存中的缓存信息写入磁盘 */
void file_flush (struct File *file)
{
    if (file == NULL)
        return;
    if (file->magic != FILE_MAGIC)
        return;
    if (file->ref <= 0)
        return;

    inode_flush(file->inode);
}

