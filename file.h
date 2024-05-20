
#ifndef __FILE_H__
#define __FILE_H__


#include "defs.h"
#include "proc.h"

struct Inode;
struct File;
struct stat;
struct statfs;
struct Dirent;
struct FileSystem;


#define INODE_MAGIC     0xDEA5
#define FILE_MAGIC      0xFE5A


/* 标记文件描述符的类型 */
enum InodeType
{
    I_NULL,        /* 空 */
    I_FILE,        /* 文件 */
    I_DIR,         /* 目录 */
    I_PIPO,        /* 管道 */
    I_DEVICE,      /* 设备 */
};


/* 文件对象的操作接口
 *
 * 设备、管道等也可以使用该结构体注册为文件对象
 */
struct FileOperation
{
    int (*open)     (struct Inode *inode);
    int (*close)    (struct Inode *inode);
    int (*ioctl)    (struct Inode *inode, int cmd, void *args);
    int (*read)     (struct Inode *inode, void *buf, unsigned int count);
    int (*write)    (struct Inode *inode, void *buf, unsigned int count);
    int (*flush)    (struct Inode *inode);
    int (*lseek)    (struct Inode *inode, bool type, unsigned int offs);
    int (*getdents) (struct Inode *inode, struct Dirent *dirp, unsigned int count);
};


/* 文件系统的操作接口 */
struct FileSystemOps
{
    /* mount and unmount file system */
    int (*mount)    (struct FileSystem *fs, unsigned long flag, void *data);
    int (*unmount)  (struct FileSystem *fs);

    /* make a file system */
    int (*mkfs)     (uint dev_id, char *fs_name);

    /* 获取文件系统的信息 */
    int (*statfs)   (struct FileSystem *fs, struct statfs *buf);

    /* 删除指定路径的文件或目录 */
    int (*unlink)   (struct FileSystem *fs, char *path);

    /* 获取文件的信息 */
    int (*stat)     (struct FileSystem *fs, char *path, struct stat *buf);

    /* 重命名指定文件 */
    int (*rename)   (struct FileSystem *fs, char *oldpath, char *newpath);

    /* 在实体文件系统中查找 inode 所对应的对象 */
    int (*lookup)   (struct FileSystem *fs, struct Inode *inode, char *path);

    /* 在实体文件系统中创建 inode 所需的对象 */
    int (*create)   (struct FileSystem *fs, struct Inode *inode, char *path);

    /* 释放在实体文件中与 inode 对应的对象 */
    int (*free)     (struct FileSystem *fs, struct Inode *inode);
};


/* 描述文件系统信息的结构体 */
struct FileSystem
{
    char                    name[20];   /* 文件系统的名字 */
    struct FileOperation    *fops;      /* 文件系统中文件的操作接口 */
    struct FileSystemOps    *fsops;     /* 文件系统的操作接口 */
    void                    *data;      /* 私有数据域 */
};


/* 管理注册到内核的每个实体文件系统
 * ( 文件系统分为注册和挂载两种模式 )
 */
struct FileSystemDev
{
    ListEntry_t          list;      /* 链接到 gFsList */
    char                 name[20];  /* 文件系统执行挂载时要设置专有名字 */
    /* 在注册模式时表示被挂载次数，在挂载模式时表示被引用次数 */
    unsigned int         ref;       /* 挂载计数/引用计数 */
    bool                 Multi;     /* 单个文件系统实体是否允许挂载多次 */
    struct FileSystem   *fs;        /* 记录传入的实体文件系统 */
};


/* 文件系统的 index node */
struct Inode
{
    uint                        magic;      /* 魔幻数 */
    struct Device              *dev;        /* 所属的设备 */
    enum InodeType              type;       /* 类型 */
    uint                        flags;      /* 文件的操作权限 */
    uint                        size;       /* 占用的磁盘大小 */
    uint                        lock;       /* 锁 */
    uint                        valid;      /* 是否已经从磁盘读取数据 */
    uint                        addr;       /* 存放磁盘地址的缓冲区 */
    uint                        writeoff;   /* 文件打开后写指针的偏移 */
    uint                        readoff;    /* 文件打开后读指针的偏移 */
    struct FileOperation       *fops;       /* 文件系统对象的操作接口 */
    struct FileSystem          *fs;         /* 当前 inode 所属的文件系统 */
    void                       *data;       /* 私有数据域(例如用于：记录实体文件系统的文件操作结构体) */
};


/* 文件描述符 */
struct File
{
    uint                magic;      /* 魔幻数 */
    uint                ref;        /* 引用计数 */
    uint                flags;      /* 文件可操作权限 */
    struct Inode       *inode;      /* 占有的 inode */
};


/* 目录项 */
struct Dirent
{
    enum InodeType type;    /* 接收到的文件类型 */
    uint namelen;           /* 实体文件系统支持的文件名长度 */
    uint objsize;           /* 接收到的单个对象的长度 */
    char name[128];         /* 文件的名字 */
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

/**********************************/
struct File *alloc_fileobj (void);
void free_fileobj  (struct File *file);
int  file_open  (struct File *file, char *path, uint flags);
int  file_close (struct File *file);
int  file_read  (struct File *file, void *buf, uint len);
int  file_write (struct File *file, void *buf, uint len);
int  file_flush (struct File *file);

/**********************************/
struct Inode *path_parser (char *path);
int path_init (void);

/**********************************/
struct Dirent *dir_open (char *name);
int dir_close (struct Dirent *dir);

/**********************************/
int fsdev_register (char *name, struct FileOperation *fops,
        struct FileSystemOps *fsops, unsigned int multi);
int fsdev_mount (char *fsname, char *mount_name,
        unsigned int flag, void *data);
int fsdev_unmount (char *name);
struct FileSystem *fsdev_get (char *name);
void fsdev_put (char *name);

#endif
