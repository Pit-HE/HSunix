/*
 * 管理并操作进程的文件描述符数组
 */
#include "fcntl.h"
#include "defs.h"


/* 用于进程申请默认的文件描述符表
 *
 * 返回值：-1为失败，非负值为正常
 */
int fdTab_alloc (ProcCB *pcb)
{
    int ret = -1;

    pcb->fdTab = (struct File **)kalloc(sizeof(struct File *) * 20);
    if (pcb->fdTab == NULL)
        return ret;
    pcb->fdCnt = 20;
    ret = 1;

    /* 标准输入 */
    pcb->fdTab[STD_INPUT] = file_alloc();
    file_open(pcb->fdTab[STD_INPUT], "console", O_RDONLY, S_IRWXU);

    /* 标准输出 */
    pcb->fdTab[STD_OUTPUT] = file_alloc();
    file_open(pcb->fdTab[STD_OUTPUT], "console", O_WRONLY, S_IRWXU);

    /* 标准错误 */
    pcb->fdTab[STD_ERROR] = file_alloc();
    file_open(pcb->fdTab[STD_ERROR], "console", O_RDONLY | O_WRONLY, S_IRWXU);

    return ret;
}
/* 释放进程占用的文件描述符表 */
void fdTab_free (ProcCB *pcb)
{
    int i;

    /* 释放数组中拥有的文件描述符 */
    for(i=0; i<pcb->fdCnt; i++)
    {
        if (pcb->fdTab[i] != NULL)
        {
            file_free(pcb->fdTab[i]);
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
    for(i=0; i<pcb->fdCnt; i++)
    {
        if (pcb->fdTab[i] == NULL)
        {
            /* 为空闲的描述数组成员分配空的描述符结构体 */
            pcb->fdTab[i] = file_alloc();
            fd = i;
            break;
        }
    }

    /*** 已无可用数组元素，则扩张进程的文件描述符指针数组 ***/
    if (fd == -1)
    {
        /* 申请新的文件描述符指针数组 */
        tab = (struct File **)kalloc(sizeof(struct File*)*(pcb->fdCnt + 5));
        if (tab != NULL)
        {
            /* 为空闲的描述数组成员分配空的描述符结构体 */
            pcb->fdTab[pcb->fdCnt] = file_alloc();
            fd = pcb->fdCnt;

            /* 记录原有的描述符信息 */
            kDISABLE_INTERRUPT();
            kmemcpy(tab, pcb->fdTab, pcb->fdCnt);
            kfree(pcb->fdTab);

            pcb->fdTab = tab;
            pcb->fdCnt += 5;
            kENABLE_INTERRUPT();
        }
    }

    return fd;
}

/* 释放申请的文件描述符数组的成员 */
void fd_free (int fd)
{
    ProcCB *pcb = getProcCB();

    if ((fd < 0) || (fd > pcb->fdCnt))
        return;

    file_free(pcb->fdTab[fd]);
    pcb->fdTab[fd] = NULL;
}

/* 通过文件描述符编号获得对应的结构体
 *
 * 返回值：NULL 为失败
 */
struct File *fd_get (int fd)
{
    ProcCB *pcb = getProcCB();

    if ((fd < 0) || (fd > pcb->fdCnt))
        return NULL;

    return pcb->fdTab[fd];
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

