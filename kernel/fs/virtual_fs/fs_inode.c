
#include "defs.h"
#include "file.h"
#include "fcntl.h"
#include "device.h"
#include "kobject.h"
#include "char_dev.h"


/*
 * 内部接口，申请与释放 inode 对象的内存空间
 */
static struct Inode *inode_alloc (void)
{
    struct Inode *inode = NULL;

    inode = (struct Inode *)kalloc(sizeof(struct Inode));
    if (inode == NULL)
        return NULL;

    inode->magic = INODE_MAGIC;

    return inode;
}
static void inode_free (struct Inode *inode)
{
    if (inode == NULL)
        return;
    if (inode->ref != 0)
        return;

    kfree(inode);
}



/* 初始化 inode 为文件系统的情况 */
struct Inode *inode_getfs (struct FsDevice *fsdev, uint flag, uint mode)
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
struct Inode *inode_getdev (struct Device *dev, uint flag, uint mode)
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

/* 初始化 inode 为 pipe 的情况 */
struct Inode *inode_getpipe (struct pipe_t *pipe, uint flag, uint mode)
{
    struct Inode *inode = NULL;

    if (pipe == NULL)
        return NULL;
    
    inode = inode_alloc();
    if (inode == NULL)
        return NULL;
    
    inode->flags = flag;
    inode->data  = (void *)pipe;
    inode->fops  = NULL;
    inode->mode  = mode;
    inode->dev   = NULL;
    inode->fs    = NULL;
    inode->ref  += 1;

    inode->type  = INODE_PIPO;

    return inode;
}

/* 初始化 inode 为字符设备的情况 */
struct Inode *inode_getcdev (struct cdev *cdev, uint flag, uint mode)
{
    struct Inode *inode = NULL;

    if (cdev == NULL)
        return NULL;
    
    inode = inode_alloc();
    if (inode == NULL)
        return NULL;
    
    inode->flags = flag;
    inode->data  = (void *)cdev;
    inode->fops  = NULL;
    inode->mode  = mode;
    inode->dev   = NULL;
    inode->fs    = NULL;
    inode->ref  += 1;

    extern struct FileOperation def_cdev_fops;
    inode->i_cdev = cdev;
    inode->fops  = &def_cdev_fops;
    list_add_after(&inode->i_devices, &cdev->list);

    inode->type  = INODE_CHRDEV;

    return inode;
}

/* 释放已经存在的 inode 节点 */
int inode_put (struct Inode *inode)
{
    if (inode == NULL)
        return -1;
    if (inode->magic != INODE_MAGIC)
        return -1;
    
    if (inode->ref > 1)
    {
        inode->ref -= 1;
        return -1;
    }

    inode->ref = 0;
    inode_free(inode);

    return 0;
}

