
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

struct InodeOperation
{
    int (*open)     (struct Inode *inode);
    int (*close)    (struct Inode *inode);
    int (*ioctl)    (struct Inode *inode, int cmd, void *args);
    int (*read)     (struct Inode *inode, void *buf, unsigned int count);
    int (*write)    (struct Inode *inode, void *buf, unsigned int count);
    int (*flush)    (struct Inode *inode);
    int (*lseek)    (struct Inode *inode, unsigned int offset);
};


/* 文件对象的操作接口
 *
 * 设备、管道等也可以使用该结构体注册为文件对象
 */
struct FileOperation
{
    int (*open)     (struct File *file);
    int (*close)    (struct File *file);
    int (*ioctl)    (struct File *file, int cmd, void *args);
    int (*read)     (struct File *file, void *buf, unsigned int count);
    int (*write)    (struct File *file, void *buf, unsigned int count);
    int (*flush)    (struct File *file);
    int (*lseek)    (struct File *file, unsigned int offset);
};


/* 文件系统的操作接口 */
struct FileSystemOps
{
    struct FileOperation *i_opss;

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
    struct Device              *dev;        /* 所属的设备 */
    enum InodeType              type;       /* 类型 */
    uint                        ref;        /* 引用计数 */
    uint                        size;       /* 占用的磁盘大小 */
    uint                        lock;       /* 锁 */
    uint                        valid;      /* 是否已经从磁盘读取数据 */
    uint                        addr;       /* 存放磁盘地址的缓冲区 */
    struct InodeOperation      *i_ops;      /* inode 对象操作接口 */
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


/* 目录项 */
struct Dirent
{
    uint flag;
    char name[16];
};


/**********************************/
int  fdTab_alloc (ProcCB *pcb);
void fdTab_free  (ProcCB *pcb);
int  fd_alloc    (void);
void fd_free     (int fd);
struct File *fd_get (int fd);
int  fd_copy     (int fd);

/**********************************/
struct Inode *inode_alloc (void);
void inode_free  (struct Inode *inode);
int  inode_open  (struct Inode *inode);
int  inode_close (struct Inode *inode);
int  inode_read  (struct Inode *inode, void *buf, uint len);
int  inode_write (struct Inode *inode, void *buf, uint len);
int  inode_flush (struct Inode *inode);

/**********************************/
struct File *file_alloc (void);
void file_free  (struct File *file);
int  file_open  (struct File *file, char *path, uint flags);
int  file_close (struct File *file);
int  file_read  (struct File *file, void *buf, uint len);
int  file_write (struct File *file, void *buf, uint len);
void file_flush (struct File *file);

/**********************************/
int path_parser (char *path, struct Inode *inode);
int path_parent (char *path, struct Inode *inode);

/**********************************/
struct Dirent *dir_open (char *name);
int dir_close (struct Dirent *dir);

#endif
