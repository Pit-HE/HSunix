/*
 * 存放磁盘文件系统内部的一些信息
 */
#ifndef __DFS_PRIV_H__
#define __DFS_PRIV_H__

#include "list.h"
#include "dfs_virtio.h"


/* 用于缓存磁盘信息的结构体 */
struct Iobuf
{
    ListEntry_t list;       /* 双向链表 */
    uchar       data[BSIZE];/* 存储磁盘信息的数组 */
    uint        ref;        /* 引用计数 */
    uint        blknum;     /* 对应的磁盘块编号 */
    bool        valid;      /* 是否已经从磁盘获取数据 */
};









/********************* dfs_iobuf **********************/
void init_iobuf (void);
void iob_write (struct Iobuf *buf);
struct Iobuf *iob_read (uint blknum);
void iob_release (struct Iobuf *buf);
void iob_get (struct Iobuf *buf);
void iob_put (struct Iobuf *buf);

/********************* dfs_virtio *********************/
void virtio_disk_io(uint blknum, uchar *data, io_type write);
void virtio_disk_isr(void);
void virtio_disk_init(void);



#endif
