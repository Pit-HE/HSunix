/*
 * 存放磁盘文件系统内部的一些信息
 */
#ifndef __DFS_PRIV_H__
#define __DFS_PRIV_H__

#include "list.h"
#include "dfs_virtio.h"

// Disk layout:
// [ boot block | super block | log | inode blocks | free bit map | data blocks]

struct Iobuf;
struct dinode;
struct superblock;


#define DFS_MAGIC   0x10203040
#define NDIRECT     12
// Bitmap bits per block
#define BPB (BSIZE * 8)
// Block of free map containing bit for block b
#define BBLOCK(b, sb) ((b)/BPB + sb.bmapstart)
// Block containing inode i
#define IBLOCK(i, sb) ((i) / (BSIZE / sizeof(struct dinode)) + sb.inodestart)


/* 用于缓存磁盘信息的结构体 */
struct Iobuf
{
    ListEntry_t list;       /* 双向链表 */
    uchar       data[BSIZE];/* 存储磁盘信息的数组 */
    uint        ref;        /* 引用计数 */
    uint        blknum;     /* 对应的磁盘块编号 */
    bool        valid;      /* 是否已经从磁盘获取数据 */
};

/* 管理磁盘超级块格式的结构体 */
struct superblock
{
  uint magic;      // Must be FSMAGIC
  uint size;       // Size of file system image (blocks)
  uint nblocks;    // Number of data blocks
  uint ninodes;    // Number of inodes.
  uint nlog;       // Number of log blocks
  uint logstart;   // Block number of first log block
  uint inodestart; // Block number of first inode block
  uint bmapstart;  // Block number of first free map block
};

/* 描述磁盘上的 inode 节点属性 */
struct dinode
{
  short type;           // File type
  short major;          // Major device number (T_DEVICE only)
  short minor;          // Minor device number (T_DEVICE only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT];  // Data block addresses
  uint extend_addr;     // 当磁盘需要扩展时，记录新扩展磁盘块的编号
};



/********************* dfs_block ***********************/
struct superblock *dsb_read (void);
void dsb_write (void);
struct dinode *dinode_alloc (uint type);
void dinode_updata (struct dinode *dnode);
uint dbmap_alloc (void);
void dbmap_free  (uint blknum);
void dblk_zero   (uint blknum);
uint dblk_write  (uint blknum, char *data, uint len);
uint dblk_read   (uint blknum, char *data, uint len);

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
