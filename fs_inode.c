
#include "defs.h"
#include "file.h"
#include "fcntl.h"


struct Inode *inode_alloc (void)
{
    struct Inode *inode = NULL;

    inode = (struct Inode *)kalloc(sizeof(struct Inode));
    if (inode == NULL)
        return NULL;

    inode->magic = INODE_MAGIC;

    return inode;
}

void inode_free (struct Inode *inode)
{
    if (inode == NULL)
        return;
    if ((inode->magic != INODE_MAGIC) ||
        (inode->ref != 0))
        return;

    kfree(inode);
}

/* 初始化 inode 为文件系统的情况 */
struct Inode *inode_setfs (struct FsDevice *fsdev,
        unsigned int flag, unsigned int mode)
{
    struct Inode *inode = NULL;

    if (fsdev == NULL)
        return NULL;
    
    inode = inode_alloc();
    if (inode == NULL)
        return NULL;

    inode->flags = flag;
    inode->fops  = fsdev->fs->fops;
    inode->mode  = mode;
    inode->dev   = NULL;
    inode->fs    = fsdev->fs;
    inode->ref  += 1;

    if (flag & O_DIRECTORY)
        inode->type = INODE_DIR;
    else
        inode->type = INODE_FILE;

    return inode;
}

/* 初始化 inode 为内核设备的情况 */
struct Inode *inode_setdev (struct Device *dev,
        unsigned int flag, unsigned int mode)
{
    struct Inode *inode = NULL;

    if (dev == NULL)
        return NULL;
    
    inode = inode_alloc();
    if (inode == NULL)
        return NULL;
    
    inode->flags = flag;
    inode->fops  = dev->opt;
    inode->mode  = mode;
    inode->dev   = dev;
    inode->fs    = NULL;
    inode->ref  += 1;

    inode->type  = INODE_DEVICE;

    return inode;
}


int inode_lookup (struct FsDevice *fsdev,
        struct Inode *inode, char *path, unsigned flag)
{
    int ret;

    if ((fsdev == NULL) || (inode == NULL) || (path == NULL))
        return -1;

    /* 查找与路径匹配的 inode */
    ret = fsdev->fs->fsops->lookup(fsdev, inode, path);
    if (ret < 0)
    {
        /* 是否要创建新的 inode */
        if ((flag & O_CREAT) != O_CREAT)
            return -1;

        /* 在磁盘上创建对应的 inode 对象 */
        ret = fsdev->fs->fsops->create(fsdev, inode, path);
    }
    return ret;
}
