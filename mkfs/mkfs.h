
#ifndef __MAKE_FS_H__
#define __MAKE_FS_H__


#ifndef NULL
#define NULL        (void*)0
#endif
#ifndef TRUE
#define TRUE        1
#endif
#ifndef FALSE
#define FALSE       0
#endif


typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;

typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long  uint64;

typedef unsigned char  bool;
typedef unsigned long  pde_t;
typedef unsigned long  size_t;

#include "list.h"



#define MAXOPBLOCKS     10  // max # of blocks any FS op writes
#define LOGSIZE         (MAXOPBLOCKS*3)  // max data blocks in on-disk log
#define NBUF            (MAXOPBLOCKS*3)  // size of disk block cache
#define FSSIZE          2000  // size of file system in blocks
#define IPB             (BSIZE / sizeof(struct disk_inode))
#define ROOTINO         1  // root i-number
#define DIRSIZ          14
#define mkIBLOCK(i,sb)  ((i) / IPB + sb.inodestart)
#define NINDIRECT       (BSIZE / sizeof(uint))
#define MAXFILE         (NDIRECT + NINDIRECT)
#define BSIZE           1024
#define NDIRECT         12
#define DFS_MAGIC       0x10203040
#define DISK_DIR        1   // Directory
#define DISK_FILE       2   // File
#define DISK_DEVICE     3   // Device


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
  short inum;          // Number of links to inode in file system
  uint size;            // Size of file (bytes)
  uint addrs[NDIRECT];  // Data block addresses
  uint ex_addr;         // 当磁盘需要扩展时，记录新扩展磁盘块的编号
};

/* 磁盘目录项，记录磁盘中的索引节点的名字与偏移值 */
struct disk_dirent
{
  /* 记录节点在磁盘索引分区中的偏移值 (0 时表示空闲) */
  ushort inum;

  /* 记录磁盘索引节点的名字 */
  char name[14];
};






#endif
