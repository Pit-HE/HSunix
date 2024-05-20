/*
 *       管理并操作文件描述符对象
 * ( 当前文件也负责处理部分 inode 的功能 )
 */
#include "defs.h"
#include "file.h"
#include "fcntl.h"


/* 申请一个文件描述符的内存空间
 *
 * 返回值：NULL 为失败
 */
struct File *alloc_fileobj (void)
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
void free_fileobj (struct File *file)
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
 * 返回值：-1表示失败
 */
int file_open (struct File *file, char *path, uint flags)
{
    int ret = 0;
    struct Device *dev;
    struct Inode *inode;

    if ((file == NULL) || (path == NULL))
        return -1;
    if (file->magic != FILE_MAGIC)
        return -1;

    /* 判断要操作的对象类型, 设备还是文件系统 */
    if ((*path == '/') || (*path == '.'))
    {
        inode = path_parser(path, flags, I_FILE);
        if (inode == NULL)
            return -1;
    }
    else
    {
        inode = inode_alloc();
        if (inode == NULL)
            return -1;

        dev = dev_get(path);
        if (dev == NULL)
            return -1;
        inode->dev  = dev;

        inode_init(inode, flags, &dev->opt, I_DEVICE);
    }

    /* 初始化新打开的文件描述符 */
    file->inode  = inode;
    file->ref   += 1;
    file->flags  = flags;

    if (inode->fops->open != NULL)
        ret = inode->fops->open(inode);

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

    if (file == NULL)
        return -1;
    if ((file->ref <= 0) || (file->magic != FILE_MAGIC))
        return -1;

    file->ref -= 1;

    inode = file->inode;
    if ((inode == NULL) || (inode->magic != INODE_MAGIC))
        return -1;

    if (inode->fops->close != NULL)
        ret = inode->fops->close(inode);

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
    int ret = 0;
    struct Inode *inode;

    if ((file == NULL) || (buf == NULL))
        return -1;
    if ((file->ref <= 0) || (file->magic != FILE_MAGIC))
        return -1;

    if ((file->flags != O_RDONLY) &&
        ((file->flags & O_ACCMODE) != O_RDWR))
        return -1;

    inode = file->inode;
    if ((inode == NULL) || (inode->magic != INODE_MAGIC))
        return -1;

    if (inode->fops->read != NULL)
        ret = inode->fops->read(inode, buf, len);

    return ret;
}

/* 将数据写入文件
 *
 * 返回值：-1表示失败，其他值表示实际写入的长度
 */
int file_write (struct File *file, void *buf, uint len)
{
    int ret = 0;
    struct Inode *inode;

    if ((file == NULL) || (buf == NULL))
        return -1;
    if ((file->ref <= 0) || (file->magic != FILE_MAGIC))
        return -1;

    if ((file->flags != O_WRONLY) &&
        ((file->flags & O_ACCMODE) != O_RDWR))
        return -1;

    inode = file->inode;
    if ((inode == NULL) || (inode->magic != INODE_MAGIC))
        return -1;

    if (inode->fops->write != NULL)
        ret = inode->fops->write(inode, buf, len);

    return ret;
}

/* 将文件在内存中的缓存信息写入磁盘
 *
 * 返回值：-1表示失败
 */
int file_flush (struct File *file)
{
    int ret;
    struct Inode *inode;

    if (file == NULL)
        return -1;
    if ((file->ref <= 0) || (file->magic != FILE_MAGIC))
        return -1;

    inode = file->inode;
    if ((inode == NULL) || (inode->magic != INODE_MAGIC))
        return -1;

    if (inode->fops->flush != NULL)
        ret = inode->fops->flush(inode);

    return ret;
}

/* 设置进程的工作路径 (传入的必须是绝对路径)
 *
 * 返回值：-1表示失败
 */
int file_setpwd (ProcCB *pcb, char *path)
{
    if ((pcb == NULL) || (path == NULL))
        return -1;

    /* 获取文件路径所对应的inode */
    pcb->pwd->inode = path_parser(path, O_RDWR,I_FILE);
    if (pcb->pwd->inode == NULL)
        return -1;

    return 0;
}
