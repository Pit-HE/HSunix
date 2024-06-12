/*
 * 存放磁盘文件系统内部的一些信息
 *
 * Disk layout:
 *      [boot block | super block | inode blocks | free bit map | data blocks]
 */
#ifndef __DFS_PRIV_H__
#define __DFS_PRIV_H__

#include "list.h"
#include "dfs_virtio.h"


#define DISK_DIR     1   // Directory
#define DISK_FILE    2   // File
#define DISK_DEVICE  3   // Device

#define DFS_MAGIC   0x10203040
#define NDIRECT     12
#define NINDIRECT   (BSIZE / sizeof(uint))

// Block of free map containing bit for block b
#define BBLOCK(b, sb) ((b)/(BSIZE * 8) + sb->bmapstart)
// Block containing inode i
#define IBLOCK(i, sb) ((i) / (BSIZE / sizeof(struct disk_inode)) + sb->inodestart)


/* 磁盘缓存区，用于和磁盘进行数据交互 */
struct disk_buf
{
    ListEntry_t list;       /* 双向链表 */
    uchar       data[BSIZE];/* 存储磁盘信息的数组 */
    uint        ref;        /* 引用计数 */
    uint        blknum;     /* 对应的磁盘块编号 */
    bool        valid;      /* 是否已经从磁盘获取数据 */
};

/* 磁盘超级块，记录整个磁盘各个分区的信息 */
struct disk_sb
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

/* 磁盘索引节点，描述磁盘中的文件对象的属性 */
struct disk_inode
{
  short type;           // File type
  short major;          // Major device number (DISK_DEVICE only)
  short minor;          // Minor device number (DISK_DEVICE only)
  short nlink;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT];  // Data block addresses
  uint ex_addr;         // 当磁盘需要扩展时，记录新扩展磁盘块的编号
};

/* 磁盘目录项，记录磁盘中的索引节点的名字与偏移值 */
struct disk_dirent
{
  /* 记录节点在磁盘索引分区中的偏移值 (0 时表示空闲) */
  uint inum;

  /* 记录磁盘索引节点的名字 */
  char name[14];
};

/********************* dfs_inode ***********************/
uint dinode_get   (uint type);
void dinode_put   (struct disk_inode *dnode);
void dinode_free  (struct disk_inode *dnode);
void dinode_flush (struct disk_inode *dnode);
void dinode_clear (struct disk_inode *dnode);
int  dinode_read  (struct disk_inode *dnode, char *dst, uint off, uint n);
int  dinode_write (struct disk_inode *dnode, char *src, uint off, uint n);
struct disk_inode *dinode_alloc   (uint inum);
struct disk_inode* dinode_parent  (char *path, char *name);
struct disk_inode* dinode_getroot (void);

/********************* dfs_dirent ***********************/
int  ddir_read   (struct disk_inode *dnode, struct disk_dirent *dir, char *name);
int  ddir_write  (struct disk_inode *dnode, char *name, uint inum);
void ddir_put    (struct disk_inode *dnode, struct disk_dirent *dir);
void ddir_flush  (struct disk_inode *dnode, struct disk_dirent *dir);
void ddir_rename (struct disk_inode *dnode, struct disk_dirent *dir, char *name);
void ddir_release(struct disk_inode *dnode, struct disk_dirent *dir);
int  ddir_check  (struct disk_inode *dnode, char *name);
int  ddir_check  (struct disk_inode *dnode, char *name);
struct disk_dirent *ddir_get (struct disk_inode *dnode, char *name);

/********************* dfs_priv ***********************/
uint  bmap (struct disk_inode *dnode, uint bn);
char *disk_path_getfirst (char *path, char *name);
char *disk_path_getlast  (char *path, char *name);
uint  dbmap_alloc (void);
void  dbmap_free  (uint blknum);
void  dblk_zero (uint blknum);
void  dsb_put (void);
struct disk_sb *dsb_get (void);

/********************* dfs_iobuf **********************/
void init_iobuf (void);
void iob_flush  (struct disk_buf *buf);
void iob_free (struct disk_buf *buf);
void iob_get  (struct disk_buf *buf);
void iob_put  (struct disk_buf *buf);
struct disk_buf *iob_alloc (uint blknum);

/********************* dfs_virtio *********************/
void virtio_disk_io  (uint blknum, uchar *data, io_type write);
void virtio_disk_isr (void);
void virtio_disk_init(void);


#endif
