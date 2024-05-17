
#ifndef __FILE_H__
#define __FILE_H__

#include "defs.h"
#include "proc.h"

struct Inode;
struct File;
struct FileSystem;

#define FILE_MAGIC  0xFD5A

/* 标记文件描述符的类型 */
enum InodeType
{
    I_NULL,        /* 空 */
    I_FILE,        /* 文件 */
    I_DIR,         /* 目录 */
    I_PIPO,        /* 管道 */
    I_DEVICE,      /* 设备 */
};

/* 文件的操作接口 */
struct FileOperation
{
    int (*open)     (struct File *fd);
    int (*close)    (struct File *fd);
    int (*ioctl)    (struct File *fd, int cmd, void *args);
    int (*read)     (struct File *fd, void *buf, uint count);
    int (*write)    (struct File *fd, const void *buf, uint count);
    int (*flush)    (struct File *fd);
    int (*lseek)    (struct File *fd, uint offset);
};

/* 文件系统的操作接口 */
struct FileSystemOps
{
    const struct FileOperation *fops;

    /* mount and unmount file system */
    int (*mount)    (struct FileSystem *fs, unsigned long rwflag, const void *data);
    int (*unmount)  (struct FileSystem *fs);

    /* make a file system */
    int (*mkfs)     (uint dev_id, const char *fs_name);
    int (*statfs)   (struct FileSystem *fs, void *buf);
    int (*unlink)   (struct FileSystem *fs, const char *pathname);
    int (*stat)     (struct FileSystem *fs, const char *filename, void *buf);
    int (*rename)   (struct FileSystem *fs, const char *oldpath, const char *newpath);
};

/* 描述文件系统信息的结构体 */
struct FileSystem
{
    char                     name[10];  /* 文件系统的名字 */
    struct FileSystemOps    *ops;       /* 文件系统的操作接口 */
    void                    *data;      /* 私有数据域 */
};

/* 文件系统的 index node */
struct Inode
{
    uint                        dev;        /* 所属的设备 */
    enum InodeType              type;       /* 类型 */
    uint                        ref;        /* 引用计数 */
    uint                        size;       /* 占用的磁盘大小 */
    uint                        lock;       /* 锁 */
    uint                        valid;      /* 是否已经从磁盘读取数据 */
    uint                        addr;       /* 存放磁盘地址的缓冲区 */
    struct FileOperation       *fop;        /* inode 对象操作接口 */
};

/* 文件描述符 */
struct File
{
    uint                magic;      /*  */
    uint                ref;        /* 引用计数 */
    uint                offset;     /* 文件位置偏移 */
    uint                flags;      /* 文件可操作权限 */
    struct Inode       *inode;      /* 占有的 inode */
};

struct Dirent
{
    uint flag;
    char name[16];
};


/**********************************/
int  fdTab_alloc (ProcCB *pcb);
void fdTab_free (ProcCB *pcb);
int fd_alloc (void);
struct File *fd_get (int fd);
void fd_put (struct File *f);
int fd_copy (int fd);

/**********************************/
struct Inode *inode_alloc (void);
void inode_free (struct Inode *inode);
int inode_open (struct Inode *inode);
void inode_close (struct Inode *inode);
int inode_read (struct Inode *inode, void *buf, uint len);
int inode_write (struct Inode *inode, void *buf, uint len);

/**********************************/
int file_open  (struct File *f, const char *path, uint flags);
int file_close (struct File *f);
int file_read  (struct File *f, void *buf, uint len);
int file_write (struct File *f, void *buf, uint len);
int file_flush (struct File *f);

#endif
