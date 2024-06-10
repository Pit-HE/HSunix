/*
 * 存放磁盘文件系统对于磁盘操作的缓冲区模块
 * ( 按照磁盘块的大小，建立多个缓冲区来记录磁盘信息 )
 */
#include "defs.h"
#include "dfs_priv.h"


/* 记录还未与磁盘建立联系的缓存对象 */
ListEntry_t gBufIdleList;
/* 记录已经与磁盘建立联系的缓存对象 */
ListEntry_t gBufDataList;



/* 目前暂时创建 20 个缓存对象用于管理 */
void init_iobuf (void)
{
    int i;
    struct Iobuf *buf = NULL;

    i = sizeof(struct Iobuf);

    /* 初始化两个结构体链表 */
    list_init(&gBufIdleList);
    list_init(&gBufDataList);

    /* 创建可用的磁盘块对象 */
    for (i=0; i<20; i++)
    {
        buf = (struct Iobuf *)kalloc(sizeof(struct Iobuf));
        if (buf == NULL)
            continue;

        buf->ref = 0;
        buf->valid = FALSE;
        buf->blknum = 0;

        list_init(&buf->list);
        list_add_after(&gBufIdleList, &buf->list);
    }
}

/* 将缓冲区数据写入磁盘 */
void iob_write (struct Iobuf *buf)
{
    if (buf == NULL)
        return;

    /* 直接将缓冲区数据写入磁盘 */
    virtio_disk_io(buf->blknum, buf->data, io_write);
}

uchar iob_tmp_sb_buff[2048];
/* 获取指定磁盘块内缓存的数据 */
struct Iobuf *iob_read (uint blknum)
{
    ListEntry_t *list = NULL;
    struct Iobuf *buf = NULL;

    /* 遍历当前数组，该磁盘块是否已经缓存 */
    list_for_each (list, &gBufDataList)
    {
        buf = list_container_of(list, struct Iobuf, list);
        if ((buf == NULL) || (buf->blknum == blknum))
            break;
    }

    /* 若没有与磁盘块对应的缓冲对象，则创建它 */ 
    if ((buf == NULL) || (buf->blknum != blknum))
    {
        /* 空闲缓冲器对象是否为空 */
        if (list_empty(&gBufIdleList))
            return NULL;
        
        /* 从空闲链表获取可用对象，并将其移除 */
        list = gBufDataList.next;
        list_del(list);
        
        /* 初始化该缓冲区对象 */
        buf = list_container_of(list, struct Iobuf, list);
        buf->ref = 0;
        buf->blknum = blknum;
        buf->valid = FALSE;

        /* 将缓冲区对象添加到数据链表中 */
        list_add_after(&gBufDataList, list);
    }

    /* 缓冲区内是否有磁盘块的数据 */
    if (buf->valid == FALSE)
    {
        virtio_disk_io(buf->blknum, buf->data, io_read);
        buf->valid = TRUE;
    }

    return buf;
}

/* 释放指定的磁盘块 */
void iob_release (struct Iobuf *buf)
{
    if (buf == NULL)
        return;
    if (buf->ref == 0)
        return;

    /* 确认缓冲区对象引用计数是否为 0 */
    buf->ref -= 1;
    if (buf->ref != 0)
        return;

    /* 当缓冲区对象引用计数为空时，释放该对象 */
    list_del(&buf->list);
    list_add_after(&gBufIdleList, &buf->list);
}

/* 获取指定的缓冲区对象 */
void iob_get (struct Iobuf *buf)
{
    buf->ref += 1;
}

/* 释放指定的缓冲区对象 */
void iob_put (struct Iobuf *buf)
{
    buf->ref -= 1;
}
