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
struct disk_inode;
struct diskfs_sb;


#define T_DIR     1   // Directory
#define T_FILE    2   // File
#define T_DEVICE  3   // Device

#define DFS_MAGIC   0x10203040
#define NDIRECT     12
#define NINDIRECT   (BSIZE / sizeof(uint))
#define MAXFILE     (NDIRECT + NINDIRECT)
// Bitmap bits per block
#define BPB (BSIZE * 8)
// Block of free map containing bit for block b
#define BBLOCK(b, sb) ((b)/BPB + sb.bmapstart)
// Block containing inode i
#define IBLOCK(i, sb) ((i) / (BSIZE / sizeof(struct disk_inode)) + sb.inodestart)


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
struct diskfs_sb
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
struct disk_inode
{
  short type;           // File type
  short major;          // Major device number (T_DEVICE only)
  short minor;          // Minor device number (T_DEVICE only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT];  // Data block addresses
  uint extend_addr;     // 当磁盘需要扩展时，记录新扩展磁盘块的编号
};

/* 描述磁盘上的目录信息
 * ( 属于 dnode 中的一种数据类型 )
 */
struct disk_dirent
{
  /* 与目录项对应的 inode 节点编号,
     为 0 时表示该目录条目空闲 */
  ushort inum;

  /* 目录条目的名字 */
  char name[16];
};


/********************* dfs_block ***********************/
struct diskfs_sb *dsb_read (void);
void dsb_write (struct diskfs_sb *sb);
struct disk_inode *dinode_alloc (uint type);
void dinode_updata (struct disk_inode *dnode);
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
