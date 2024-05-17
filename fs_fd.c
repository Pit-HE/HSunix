/*
 * 提供文件系统中，操作文件描述符与进程的文件描述符数组的接口
 */
#include "file.h"


/* 申请一个文件描述符的内存空间
 *
 * 返回值：NULL 为失败
 */
static struct File *_allocFD (void)
{
    struct File *fd = NULL;

    fd = (struct File *)kalloc(sizeof(struct File));
    if (fd == NULL)
        return fd;
    memset(fd, 0, sizeof(struct File));

    fd->magic = FILE_MAGIC;
    fd->ref = 1;

    fd->inode = inode_alloc();

    return fd;
}
/* 释放已申请的文件描述符 */
static void _freeFD (struct File *fd)
{
    if (fd == NULL)
        return;
    if (fd->magic != FILE_MAGIC)
        return;
    if (--fd->ref != 0)
        return;

    inode_free(fd->inode);
    kfree(fd);
}



/* 用于进程申请默认的文件描述符表
 *
 * 返回值：-1为失败，非负值为正常
 */
int fdTab_alloc (ProcCB *pcb)
{
    int ret = -1;
    struct Inode *inode;

    pcb->fdTab = (struct File **)kalloc(sizeof(struct File *) * 20);
    if (pcb->fdTab == NULL)
        return ret;
    pcb->fdLen = 20;
    ret = 1;

    memset (pcb->fdTab, 0, sizeof(struct File *) * 20);

    /* 标准输入 */
    pcb->fdTab[0] = _allocFD();
    inode = inode_alloc();
    inode->dev = 0;// 0号设备，控制台
    inode->type = I_DEVICE;
    pcb->fdTab[0]->inode = inode;

    /* 标准输出 */
    pcb->fdTab[1] = _allocFD();
    inode = inode_alloc();
    inode->dev = 0;// 0号设备，控制台
    inode->type = I_DEVICE;
    pcb->fdTab[1]->inode = inode;

    /* 标准错误 */
    pcb->fdTab[2] = _allocFD();
    inode = inode_alloc();
    inode->dev = 0;// 0号设备，控制台
    inode->type = I_DEVICE;
    pcb->fdTab[2]->inode = inode;

    return ret;
}
/* 释放进程占用的文件描述符表 */
void fdTab_free (ProcCB *pcb)
{
    int i;

    /* 释放数组中拥有的文件描述符 */
    for(i=0; i<pcb->fdLen; i++)
    {
        if (pcb->fdTab[i] != NULL)
        {
            _freeFD(pcb->fdTab[i]);
        }
    }
    /* 释放整个文件描述符数组占用的空间 */
    kfree(pcb->fdTab);
}

/* 申请进程中空闲的文件描述符编号
 *
 * 返回值：-1为失败，非负值为正常
 */
int fd_alloc (void)
{
    int i, fd = -1;
    struct File **tab = NULL;
    ProcCB *pcb = getProcCB();

    /* 遍历文件描述符指针数组，寻找空闲的数组元素 */
    for(i=0; i<pcb->fdLen; i++)
    {
        if (pcb->fdTab[i] == NULL)
        {
            /* 为空闲的描述数组成员分配空的描述符结构体 */
            pcb->fdTab[i] = _allocFD();
            fd = i;
            break;
        }
    }

    /* 已无可用空间，则扩张进程的文件描述符指针数组 */
    if ((fd == -1) && (i == pcb->fdLen))
    {
        /* 申请新的文件描述符指针数组 */
        tab = (struct File **)kalloc(sizeof(struct File*)*(pcb->fdLen + 5));
        if (tab != NULL)
        {
            memset(tab, 0, sizeof(struct File*)*(pcb->fdLen + 5));
            /* 为空闲的描述数组成员分配空的描述符结构体 */
            pcb->fdTab[pcb->fdLen] = _allocFD();
            fd = pcb->fdLen;

            /* 记录原有的描述符信息 */
            kDISABLE_INTERRUPT();
            memcpy(tab, pcb->fdTab, pcb->fdLen);
            kfree(pcb->fdTab);

            pcb->fdTab = tab;
            pcb->fdLen += 5;
            kENABLE_INTERRUPT();
        }
    }

    return fd;
}

/* 通过文件描述符编号获得对应的结构体
 *
 * 返回值：NULL 为失败
 */
struct File *fd_get (int fd)
{
    ProcCB *pcb = getProcCB();

    if ((fd < 0) || (fd > pcb->fdLen))
        return NULL;

    pcb->fdTab[fd]->ref += 1;

    return pcb->fdTab[fd];
}

/* 释放已经获取的文件描述符 */
void fd_put (struct File *f)
{
    if (f == NULL)
        return;
    if (f->magic != FILE_MAGIC)
        return;

    _freeFD(f);
}

/* 将入参的文件描述符拷贝一个新的并返回
 *
 * 返回值：-1为失败，非负值为正常
 */
int fd_copy (int fd)
{
    int newfd;
    ProcCB *pcb;
    struct File *f;

    f = fd_get(fd);
    if (f == NULL)
        return -1;
    if (f->magic != FILE_MAGIC)
        return -1;
    if (f->ref == 0)
        return -1;

    newfd = fd_alloc();
    if (newfd < 0)
        return -1;

    pcb = getProcCB();
    pcb->fdTab[newfd] = f;

    f->ref += 1;

    return newfd;
}

