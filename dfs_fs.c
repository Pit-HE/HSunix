/*
 * 存放 disk file system 对外的功能接口
 */
#include "defs.h"
#include "dfs_priv.h"


/****************************************************
 *      dfs 文件系统文件操作接口
 ***************************************************/
int dfs_open (struct File *file)
{
    return 0;
}
int dfs_close (struct File *file)
{
    return 0;
}
int dfs_ioctl (struct File *file, int cmd, void *args)
{
    return 0;
}
int dfs_read (struct File *file, void *buf, uint count)
{
    return 0;
}
int dfs_write (struct File *file, void *buf, uint count)
{
    return 0;
}
int dfs_flush (struct File *file)
{
    return 0;
}
int dfs_lseek (struct File *file, uint offs, uint type)
{
    return 0;
}
int dfs_getdents (struct File *file, struct dirent *dirp, uint count)
{
    return 0;
}

/****************************************************
 *      dfs 文件系统文件操作接口
 ***************************************************/
int dfs_mount (struct FsDevice *fsdev, uint flag, void *data)
{
    return 0;
}
int dfs_unmount (struct FsDevice *fsdev)
{
    return 0;
}
int dfs_statfs (struct FsDevice *fsdev, struct statfs *buf)
{
    return 0;
}
int dfs_unlink (struct DirItem *ditem)
{
    return 0;
}
int dfs_stat (struct File *file, struct stat *buf)
{
    return 0;
}
int dfs_rename (struct DirItem *old_ditem, struct DirItem *new_ditem)
{
    return 0;
}
int dfs_lookup (struct FsDevice *fsdev, 
        struct Inode *inode, const char *path)
{
    return 0;
}
int dfs_create (struct FsDevice *fsdev, struct Inode *inode, char *path)
{
    return 0;
}
int dfs_free (struct FsDevice *fsdev, struct Inode *inode)
{
    return 0;
}

/****************************************************
 *      dfs 文件系统的结构体信息
 ***************************************************/
/* dfs 文件系统的文件操作接口 */
struct FileOperation dfs_fops =
{
    .open     = dfs_open,
    .close    = dfs_close,
    .ioctl    = dfs_ioctl,
    .read     = dfs_read,
    .write    = dfs_write,
    .flush    = dfs_flush,
    .lseek    = dfs_lseek,
    .getdents = dfs_getdents,
    .stat     = dfs_stat,
    .rename   = dfs_rename,
};
/* dfs 文件系统的系统操作接口 */
struct FileSystemOps dfs_fsops =
{
    .mount   = dfs_mount,
    .unmount = dfs_unmount,
    .statfs  = dfs_statfs,
    .unlink  = dfs_unlink,
    .lookup  = dfs_lookup,
    .create  = dfs_create,
    .free    = dfs_free,
};


/****************************************************
 *      dfs 文件系统对外的接口
 ***************************************************/
/* 初始化当前文件系统, 并注册到系统内核 */
void init_dfs (void)
{
    /* 虚拟磁盘初始化 */
    virtio_disk_init();

    /* 将 dfs 注册为内核文件系统对象 */
    fsobj_register("dfs", &dfs_fops, &dfs_fsops, TRUE);
}
