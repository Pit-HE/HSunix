
#ifndef __FILE_H__
#define __FILE_H__


#include "defs.h"
#include "proc.h"

struct Inode;
struct File;
struct stat;
struct statfs;
struct dirent;
struct FileSystem;
struct FsDevice;


#define INODE_MAGIC     0xDEA5
#define FILE_MAGIC      0xFE5A
#define DIRITEM_MAGIC   0xD55D

#define  FS_NAME_LEN 32



/* 标记文件描述符的类型 */
enum InodeType
{
    INODE_NULL,        /* 空 */
    INODE_FILE,        /* 文件 */
    INODE_DIR,         /* 目录 */
    INODE_PIPO,        /* 管道 */
    INODE_DEVICE,      /* 设备 */
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
    /* 同步缓存的数据到实体文件系统中 */
    int (*flush)    (struct Inode *inode);
    /* 设置文件的偏移量 */
    int (*lseek)    (struct Inode *inode, bool type, unsigned int offs);
    /* 获取目录信息 */
    int (*getdents) (struct Inode *inode, struct dirent *dirp, unsigned int count);
};


/* 文件系统的操作接口 */
struct FileSystemOps
{
    /* mount and unmount file system */
    int (*mount)    (struct FsDevice *fsdev, unsigned long flag, void *data);
    int (*unmount)  (struct FsDevice *fsdev);

    /* make a file system */
    int (*mkfs)     (unsigned int dev_id, char *fs_name);

    /* 获取文件系统的信息 */
    int (*statfs)   (struct FsDevice *fsdev, struct statfs *buf);

    /* 删除指定路径的文件或目录 */
    int (*unlink)   (struct FsDevice *fsdev, char *path);

    /* 获取文件的信息 */
    int (*stat)     (struct FsDevice *fsdev, char *path, struct stat *buf);

    /* 重命名指定文件 */
    int (*rename)   (struct FsDevice *fsdev, char *oldpath, char *newpath);

    /* 在实体文件系统中查找 inode 所对应的对象 */
    int (*lookup)   (struct FsDevice *fsdev, struct Inode *inode, char *path);

    /* 在实体文件系统中创建 inode 所需的对象 */
    int (*create)   (struct FsDevice *fsdev, struct Inode *inode, char *path);

    /* 释放在实体文件中与 inode 对应的对象 */
    int (*free)     (struct FsDevice *fsdev, struct Inode *inode);
};


/* 描述文件系统信息的结构体 */
struct FileSystem
{
    char                     name[FS_NAME_LEN];
    struct FileOperation    *fops;      /* 文件系统中文件的操作接口 */
    struct FileSystemOps    *fsops;     /* 文件系统的操作接口 */
};


/* 管理注册到内核的每个实体文件系统
 */
struct FsDevice
{
    ListEntry_t          list;      /* 链接到 gFsDevList */
    char                *path;      /* 文件系统挂载的路径 */
    /* 在注册模式时表示被挂载次数，在挂载模式时表示被引用次数 */
    unsigned int         ref;       /* 挂载计数/引用计数 */
    struct FileSystem   *fs;        /* 记录传入的实体文件系统 */
    void                *data;      /* 私有数据域，存放临时的数据 */
};


/*         文件系统的 index node 
 * ( 主要作用是记录实体文件系统对象的信息 )
 */
struct Inode
{
    unsigned int                magic;      /* 魔幻数 */
    struct Device              *dev;        /* 操作的设备 */
    enum InodeType              type;       /* 类型 */
    unsigned int                flags;      /* 操作类型 */
    unsigned int                mode;       /* 访问的权限 */
    unsigned int                size;       /* 占用的磁盘大小 */
    unsigned int                lock;       /* 锁 */
    unsigned int                valid;      /* 是否已经从磁盘读取数据 */
    unsigned int                addr;       /* 存放磁盘地址的缓冲区 */
    unsigned int                ref;        /* 引用计数 */
    unsigned int                writeoff;   /* 文件打开后写指针的偏移 */
    unsigned int                readoff;    /* 文件打开后读指针的偏移 */
    struct FileOperation       *fops;       /* 文件对象的操作接口 */
    struct FileSystem          *fs;         /* 所属的文件系统 */
    void                       *data;       /* 私有数据域(例如用于：记录实体文件系统的文件操作结构体) */
};


/* 文件描述符 */
struct File
{
    unsigned int            magic;  /* 魔幻数 */
    unsigned int            ref;    /* 引用计数 */
    unsigned int            flags;  /* 文件可操作权限 */
    struct FileOperation   *fops;   /* 文件对象的操作接口 */
    struct Inode           *inode;  /* 占有的 inode */
    struct DirItem         *ditem;  /* 占有的目录项 */
};


/*    file system directory item
 *    虚拟文件系统内部使用的目录项
 * ( 主要作用是记录 inode 对象的路径 )
 */
struct DirItem
{
    unsigned int          magic;    /* 魔幻数 */
    ListEntry_t           list;     /* 用于挂载的链表对象 */
    enum{
        DITEM_MOUNT = 0x01,
        DITEM_ALLOC = 0x02,
        DITEM_HASH  = 0x04,
        DITEM_OPEN  = 0x08,
        DITEM_CLOSE = 0x10,
    }state;                         /* 状态 */
    unsigned int          ref;      /* 引用计数 */
    /* 相对路径 (不包含所属文件系统的挂载路径) */
    char                 *path;
    struct Inode         *inode;    /* 对应的 inode 节点 */
    struct FsDevice      *fsdev;    /* 所属的已挂载文件系统 */
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
void inode_free (struct Inode *inode);
int inode_init (struct Inode *inode, unsigned int flag,
        struct FileOperation *fops, unsigned int mode);

/**********************************/
struct File *file_alloc (void);
void file_free   (struct File *file);
int  file_open      (struct File *file, char *path, 
    unsigned int flags, unsigned int mode);
int  file_close     (struct File *file);
int  file_read      (struct File *file, 
    void *buf, unsigned int len);
int  file_write     (struct File *file, 
    void *buf, unsigned int len);
int  file_flush     (struct File *file);

/**********************************/
char *path_getfirst (char *path, char *name);
int path_getlast (char *path, char *parentPath, char *name);
struct Inode *parse_getinode (char *path,
    unsigned int flag, unsigned int mode);
char *path_formater (char *path);
char *path_parser (char *directory, char *filepath);
int path_setcwd (char *path);
char *path_getcwd (void);

/**********************************/
void init_ditem (void);
struct DirItem *ditem_alloc (
        struct FsDevice *fsdev, char *path);
int  ditem_free (struct DirItem *dir);
struct DirItem *ditem_get (struct FsDevice *fsdev, char *path,
          unsigned int flag, unsigned int mode);
void ditem_put (struct DirItem *ditem);
char *ditem_path (struct DirItem *ditem);

/**********************************/
int fsdev_register (char *name, struct FileOperation *fops,
        struct FileSystemOps *fsops, unsigned int multi);
int fsdev_mount (char *fsname, char *path,
        unsigned int flag, void *data);
int fsdev_unmount (char *path);
struct FsDevice *fsdev_get (char *path);
void fsdev_put (struct FsDevice *fsdev);

#endif
