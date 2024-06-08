/*
 * 存放磁盘文件系统内部的一些信息
 */
#ifndef __DFS_PRIV_H__
#define __DFS_PRIV_H__

#include "dfs_virtio.h"
















/********************* dfs_virtio *********************/
void virtio_disk_io(uint blockno, uchar *data, io_type write);
void virtio_disk_isr(void);
void virtio_disk_init(void);


#endif
