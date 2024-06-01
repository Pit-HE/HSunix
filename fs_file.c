/*
 *       管理并操作文件描述符对象
 * ( 当前文件也负责处理部分 inode 的功能 )
 */
#include "defs.h"
#include "file.h"
#include "fcntl.h"


/* 申请一个文件描述符的内存空间
 *
 * 返回值：NULL 为失败
 */
struct File *file_alloc (void)
{
    struct File *file = NULL;

    file = (struct File *)kalloc(sizeof(struct File));
    if (file == NULL)
        return file;

    file->magic = FILE_MAGIC;

    return file;
}

/* 释放已申请的文件描述符 */
void file_free (struct File *file)
{
    if (file == NULL)
        return;
    if (file->magic != FILE_MAGIC)
        return;
    if (file->ref != 0)
        return;

    file->magic = 0;
    kfree(file);
}

/* 打开文件系统中要操作的对象，将其信息存入文件描述符
 *
 * file: 要操作的文件对象 (用于存储当前函数获取到的信息)
 * path: 要操作的文件路径对象所在的路径 (绝对路径/相对路径)
 * flag: 要执行的操作(O_CREAT、O_RDWR、O_DIRECTORY... )
 * mode: 该文件对象的权限
 * 
 * 返回值：-1表示失败
 */
int file_open (struct File *file, char *path, 
        unsigned int flag, unsigned int mode)
{
    int ret = 0;
    char *ap_path = NULL;
    char *std_path = NULL;
    struct Device *dev = NULL;
    struct Inode *inode = NULL;
    struct DirItem *ditem = NULL;
    struct FsDevice *fsdev = NULL;
    

    if ((file == NULL) || (path == NULL))
        return -1;
    if (file->magic != FILE_MAGIC)
        return -1;

    /* 判断要操作的对象类型, 设备还是文件系统 */
    if (*path != ':')
    {
        /* 将文件路径转化为绝对路径 */
        ap_path = path_parser(NULL, path);

        /* 获取该路径下所对应的文件系统 */
        fsdev = fsdev_get(ap_path);
        if (fsdev == NULL)
            return -1;

        /* 获取该路径下所对应的目录项 */
        ditem = ditem_get(fsdev, ap_path);
        if (ditem == NULL)
        {
            /* 若不存在则创建 */
            ditem = ditem_create(fsdev, ap_path, flag, mode);
            if (ditem == NULL)
            {
                fsdev_put(fsdev);
                return -1;
            }
            /*  */
            if (ditem->inode->type == INODE_DIR)
            {
                std_path = (char *)kalloc(kstrlen(ap_path)+3);
                if (std_path == NULL)
                {
                    ditem_free(ditem);
                    fsdev_put(fsdev);
                    return -1;
                }
                kstrcpy(std_path, ap_path);

                if (ap_path[kstrlen(ap_path)-1] != '/')
                    kstrcat(std_path, "/");

                kstrcat(std_path, ".");
                ditem_create(fsdev, std_path, flag, mode);
                kstrcat(std_path, ".");
                ditem_create(fsdev, std_path, flag, mode);
                kfree(std_path);
            }
        }
        // kfree(ap_path)   /* TODO */

        inode = ditem->inode;
    }
    else /* 操作注册的设备 */
    {
        inode = inode_alloc();
        if (inode == NULL)
            return -1;

        dev = dev_get(path);
        if (dev == NULL)
        {
            inode_free(inode);
            return -1;
        }
        inode_init(inode, flag, &dev->opt, S_IRWXU);

        inode->dev  = dev;
        inode->type = INODE_DEVICE;
    }

    /* 初始化新打开的文件描述符 */
    file->inode = inode;
    file->ref  += 1;
    file->flags = flag;
    file->fops  = inode->fops;
    file->ditem = ditem;

    if (inode->fops->open != NULL)
        ret = inode->fops->open(file);

    return ret;
}

/* 关闭已打开的文件对象，注销已打开的文件描述符
 *
 * 返回值：-1表示失败
 */
int file_close (struct File *file)
{
    int ret;
    struct Inode *inode;

    if (file == NULL)
        return -1;
    if ((file->ref <= 0) || (file->magic != FILE_MAGIC))
        return -1;

    file->ref -= 1;

    inode = file->inode;
    if (inode == NULL)
        return -1;

    if (file->fops->close != NULL)
        ret = file->fops->close(file);

    /* 释放文件对象占用的资源 */
    if (inode->type == INODE_DEVICE)
    {
        dev_put(file->inode->dev);
        inode_free(file->inode);
    }
    else
    {
        fsdev_put(file->ditem->fsdev);
        ditem_put(file->ditem);
    }

    return ret;
}

/* 读取文件数据
 *
 * 返回值：-1表示失败，其他值表示实际读取的长度
 */
int file_read (struct File *file, 
    void *buf, unsigned int len)
{
    int ret = 0;

    if ((file == NULL) || (buf == NULL))
        return -1;
    if ((file->ref <= 0) || (file->magic != FILE_MAGIC))
        return -1;

    if ((file->flags != O_RDONLY) &&
        ((file->flags & O_ACCMODE) != O_RDWR))
        return -1;

    if (file->fops->read != NULL)
        ret = file->fops->read(file, buf, len);

    return ret;
}

/* 将数据写入文件
 *
 * 返回值：-1表示失败，其他值表示实际写入的长度
 */
int file_write (struct File *file, 
    void *buf, unsigned int len)
{
    int ret = 0;

    if ((file == NULL) || (buf == NULL))
        return -1;
    if ((file->ref <= 0) || (file->magic != FILE_MAGIC))
        return -1;

    if ((file->flags != O_WRONLY) &&
        ((file->flags & O_ACCMODE) != O_RDWR))
        return -1;

    if (file->fops->write != NULL)
        ret = file->fops->write(file, buf, len);

    return ret;
}

/* 将文件在内存中的缓存信息写入磁盘
 *
 * 返回值：-1表示失败
 */
int file_flush (struct File *file)
{
    int ret = 0;

    if (file == NULL)
        return -1;
    if ((file->ref <= 0) || (file->magic != FILE_MAGIC))
        return -1;

    if (file->fops->flush != NULL)
        ret = file->fops->flush(file);

    return ret;
}


/* 读取指定数量的目录信息
 *  
 * 返回值：-1表示失败, 其他表示读取到的总内存大小
 */
int file_getdents(struct File *file, 
    struct dirent *dirp, unsigned int nbytes)
{
    int ret = -1;

    if ((file == NULL) || (dirp == NULL) || 
        (nbytes == 0))
        return -1;
    if ((file->ref <= 0) || (file->magic != FILE_MAGIC))
        return -1;

    if (file->fops->getdents != NULL)
        ret = file->fops->getdents(file, dirp, nbytes);

    return ret;
}

/* 设置目录文件对象的偏移指针 */
int file_lseek (struct File *file, 
    unsigned int offset, unsigned int type)
{
    int ret = -1;

    if (file == NULL)
        return -1;
    if ((file->ref <= 0) || (file->magic != FILE_MAGIC))
        return -1;

    if (file->fops->lseek != NULL)
        ret = file->fops->lseek(file, offset, type);

    return ret;
}

