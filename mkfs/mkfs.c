#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

// #include "dfs_priv.h"
// #include "dfs_virtio.h"
// #include "param.h"
// #include "types.h"
#include "mkfs.h"



#ifndef static_assert
#define static_assert(a, b) \
  do                        \
  {                         \
    switch (0)              \
    case 0:                 \
    case (a):;              \
  } while (0)
#endif


#define NINODES 200

/*
**
  用于创建初始化的文件系统，设置管理磁盘的超级块
  按照功能项划分磁盘的各个分区
**
*/

// Disk layout:
// [ boot block | sb block | log | inode blocks | free bit map | data blocks ]

int nbitmap = FSSIZE / (BSIZE * 8) + 1;
int ninodeblocks = NINODES / IPB + 1;
int nlog = LOGSIZE;
int nmeta;   // Number of meta blocks (boot, sb, nlog, inode, bitmap)
int nblocks; // Number of data blocks

/* 记录作为磁盘的文件描述符 */
int fsfd;
/* 记录当前磁盘文件中超级块的信息 */
struct disk_sb sb;
char zeroes[BSIZE];
/* 记录磁盘文件中第一个空闲 inode 的编号 */
uint freeinode = 1;
/* 记录当前磁盘文件使用了多少个空闲块 */
uint freeblock;

void balloc(int);
void wsect(uint, void *);
void winode(uint, struct disk_inode *);
void rinode(uint inum, struct disk_inode *ip);
void rsect(uint sec, void *buf);
uint ialloc(ushort type);
void iappend(uint inum, void *p, int n);
void die(const char *);

// convert to riscv byte order
/* short 类型的大小端转化 */
ushort xshort(ushort x)
{
  ushort y;
  uchar *a = (uchar *)&y;
  a[0] = x;
  a[1] = x >> 8;
  return y;
}
/* int 类型的大小端转化 */
uint xint(uint x)
{
  uint y;
  uchar *a = (uchar *)&y;
  a[0] = x;
  a[1] = x >> 8;
  a[2] = x >> 16;
  a[3] = x >> 24;
  return y;
}

/* 将 buf 信息写入到磁盘文件指定的段中 */
void wsect(uint sec, void *buf)
{
  if (lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE)
    die("lseek");
  if (write(fsfd, buf, BSIZE) != BSIZE)
    die("write");
}

/* 将 inode 信息写入到磁盘文件指定的块中 */
void winode(uint inum, struct disk_inode *ip)
{
  char buf[BSIZE];
  uint bn;
  struct disk_inode *dip;

  bn = mkIBLOCK(inum, sb);
  rsect(bn, buf);
  dip = ((struct disk_inode *)buf) + (inum % IPB);
  *dip = *ip;
  wsect(bn, buf);
}

/* 从指定的磁盘文件块中读取 inode 信息 */
void rinode(uint inum, struct disk_inode *ip)
{
  char buf[BSIZE];
  uint bn;
  struct disk_inode *dip;

  bn = mkIBLOCK(inum, sb);
  rsect(bn, buf);
  dip = ((struct disk_inode *)buf) + (inum % IPB);
  *ip = *dip;
}

/* 将磁盘文件的指定段信息读取到 buf 中 */
void rsect(uint sec, void *buf)
{
  if (lseek(fsfd, sec * BSIZE, 0) != sec * BSIZE)
    die("lseek");
  if (read(fsfd, buf, BSIZE) != BSIZE)
    die("read");
}

/* 从磁盘文件中获取一个空闲的 inode 并初始化 */
uint ialloc(ushort type)
{
  uint inum = freeinode++;
  struct disk_inode din;

  /* 初始化该 inode  节点信息 */
  bzero(&din, sizeof(din));
  din.type = xshort(type);
  din.inum = xshort(1);
  din.size = xint(0);
  /* 将 inode 信息写入磁盘文件 */
  winode(inum, &din);
  /* 返回该 inode 在磁盘文件中的编号 */
  return inum;
}

/* 申请获取磁盘的第一个块并初始化
 * (这个区域用于记录当前磁盘所有内存块的使用情况)
 */
void balloc(int used)
{
  uchar buf[BSIZE];
  int i;

  printf("balloc: first %d blocks have been allocated\n", used);
  assert(used < BSIZE * 8);
  /* 按照文件系统格式清空该内存块 */
  bzero(buf, BSIZE);
  for (i = 0; i < used; i++)
  {
    buf[i / 8] = buf[i / 8] | (0x1 << (i % 8));
  }
  printf("balloc: write bitmap block at sector %d\n", sb.bmapstart);
  /* 更新该磁盘的位图信息 */
  wsect(sb.bmapstart, buf);
}

/* 判断哪一个更小 */
#define min(a, b) ((a) < (b) ? (a) : (b))

/* 将 xp 的内容写入到指定编号的 inode 对应磁盘文件的可用空闲块中 */
void iappend(uint inum, void *xp, int n)
{
  char *p = (char *)xp;
  uint fbn, off, n1;
  struct disk_inode din;
  char buf[BSIZE];
  uint indirect[NINDIRECT];
  uint x;

  /* 获取磁盘文件中关于当前编号的 inode */
  rinode(inum, &din);
  /* 获取当前 inode 可用的初始大小 */
  off = xint(din.size);

  // printf("append inum %d at off %d sz %d\n", inum, off, n);
  /* 将 xp 缓冲区的信息，写入到指定编号的磁盘 inode 所占用的空闲磁盘块中 */
  while (n > 0)
  {
    /* 计算当前写到了 inode 的第几个空闲块 */
    fbn = off / BSIZE;
    assert(fbn < MAXFILE);
    /* 判断要写入的信息是否已经超过 inode 初始分配的空闲内存块数量 */
    if (fbn < NDIRECT)
    {
      /* 要写入的内容未超过 inode 的初始空闲块数量 */
      if (xint(din.addrs[fbn]) == 0)
      {
        din.addrs[fbn] = xint(freeblock++);
      }
      /* 获取要操作的 inode 中的空闲块编号 */
      x = xint(din.addrs[fbn]);
    }
    else
    {
      /* 要写入的内容超过了 inode 的初始空闲块数量 */
      if (xint(din.ex_addr) == 0)
      {
        din.ex_addr = xint(freeblock++);
      }
      /* 在最后一个块建立新分配空闲内存块的索引 */
      rsect(xint(din.ex_addr), (char *)indirect);
      if (indirect[fbn - NDIRECT] == 0)
      {
        indirect[fbn - NDIRECT] = xint(freeblock++);
        wsect(xint(din.ex_addr), (char *)indirect);
      }
      /* 获取要操作的 inode 中的空闲块编号 */
      x = xint(indirect[fbn - NDIRECT]);
    }
    /* 计算要写入磁盘文件空闲空间的数据大小 */
    n1 = min(n, (fbn + 1) * BSIZE - off);

    /* 将传入的缓冲区写到磁盘文件指定的段中 */
    rsect(x, buf);
    bcopy(p, buf + off - (fbn * BSIZE), n1);
    wsect(x, buf);
    n -= n1;
    off += n1;
    p += n1;
  }
  din.size = xint(off);
  winode(inum, &din);
}

/* 发生错误时的处理 */
void die(const char *s)
{
  perror(s);
  exit(1);
}

/* 创建文件夹下的 . 和 .. */
void dir_stand (uint inum)
{
  struct disk_dirent de;

  /* 创建文件夹下的 . */
  bzero(&de, sizeof(de));
  de.inum = xshort(inum);
  strcpy(de.name, ".");
  iappend(inum, &de, sizeof(de));

  /* 创建文件夹下的 .. */
  bzero(&de, sizeof(de));
  de.inum = xshort(inum);
  strcpy(de.name, "..");
  iappend(inum, &de, sizeof(de));
}

/* 在指定的节点(inum) 下创建文件夹 */
void dir_create (uint inum, struct disk_dirent *obj, char *name)
{
  uint tmp_inm;

  tmp_inm = ialloc(DISK_DIR);

  /* 创建目标文件夹 */
  bzero(obj, sizeof(struct disk_dirent));
  obj->inum = xshort(tmp_inm);
  strncpy(obj->name, name, DIRSIZ);
  iappend(inum, obj, sizeof(struct disk_dirent));

  /* 创建文件夹下的 . 和 .. */
  dir_stand(tmp_inm);
}

uint file_create (uint inum, char *name)
{
  uint tmp;
  struct disk_dirent de;

  tmp = ialloc(DISK_FILE);

  bzero(&de, sizeof(de));
  de.inum = xshort(tmp);
  strncpy(de.name, name, DIRSIZ);
  iappend(inum, &de, sizeof(de));

  return tmp;
}

int main(int argc, char *argv[])
{
  int i, cc, fd;
  uint rootino, inum, off;
  struct disk_dirent obj;
  char buf[BSIZE];
  struct disk_inode din;

  static_assert(sizeof(int) == 4, "Integers must be 4 bytes!");

  if (argc < 2)
  {
    fprintf(stderr, "Usage: mkfs fs.img files...\n");
    exit(1);
  }

  assert((BSIZE % sizeof(struct disk_inode)) == 0);
  assert((BSIZE % sizeof(struct disk_dirent)) == 0);

  /* 创建作为磁盘的文件 */
  fsfd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0666);
  if (fsfd < 0)
    die(argv[1]);

  // 1 fs block = 1 disk sector
  nmeta = 2 + nlog + ninodeblocks + nbitmap;
  nblocks = FSSIZE - nmeta;

  sb.magic = DFS_MAGIC;
  sb.size = xint(FSSIZE);
  sb.nblocks = xint(nblocks);
  sb.ninodes = xint(NINODES);
  sb.nlog = xint(nlog);
  sb.logstart = xint(2);
  sb.inodestart = xint(2 + nlog);
  sb.bmapstart = xint(2 + nlog + ninodeblocks);

  printf("nmeta %d (boot, super, log blocks %u inode blocks %u, bitmap blocks %u) blocks %d total %d\n",
         nmeta, nlog, ninodeblocks, nbitmap, nblocks, FSSIZE);

  freeblock = nmeta; // the first free block that we can allocate

  for (i = 0; i < FSSIZE; i++)
    wsect(i, zeroes);

  /* 将超级块信息写入磁盘 */
  memset(buf, 0, sizeof(buf));
  memmove(buf, &sb, sizeof(sb));
  wsect(1, buf);

  /* 获取根目录文件夹的 inode 信息 */
  rootino = ialloc(DISK_DIR);
  assert(rootino == ROOTINO);

  /* 创建文件夹下的 . 和 .. */
  dir_stand(rootino);

  /* 创建根目录下的标准文件夹 */
  dir_create(rootino, &obj, "sys");
  dir_create(rootino, &obj, "usr");
  dir_create(rootino, &obj, "lib");
  dir_create(rootino, &obj, "dev");
  dir_create(rootino, &obj, "home");
  dir_create(rootino, &obj, "root");
  dir_create(rootino, &obj, "tmp");
  dir_create(rootino, &obj, "bin");

  /* 解析要烧录到磁盘的 elf 文件 */
  for (i = 2; i < argc; i++)
  {
    // get rid of "user/"
    char *shortname;
    if (strncmp(argv[i], "bin/", 4) == 0)
      shortname = argv[i] + 4;
    else
      shortname = argv[i];

    assert(index(shortname, '/') == 0);

    if ((fd = open(argv[i], 0)) < 0)
      die(argv[i]);

    // Skip leading _ in name when writing to file system.
    // The binaries are named _rm, _cat, etc. to keep the
    // build operating system from trying to execute them
    // in place of system binaries like rm and cat.
    if (shortname[0] == '_')
      shortname += 1;

    /* 在指定的节点下创建文件 */
    inum = file_create(obj.inum, shortname);

    /* 将数据写入到节点中 */
    while ((cc = read(fd, buf, sizeof(buf))) > 0)
      iappend(inum, buf, cc);

    close(fd);
  }

  // fix size of root inode dir
  rinode(rootino, &din);
  off = xint(din.size);
  off = ((off / BSIZE) + 1) * BSIZE;
  din.size = xint(off);
  winode(rootino, &din);

  /* 格式化当前磁盘所有块使用情况的区域 */
  balloc(freeblock);

  exit(0);
}
