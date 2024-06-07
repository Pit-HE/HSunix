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
    if ((file->magic != FILE_MAGIC) || 
        (file->ref != 0))
        return;

    file->magic = 0;
    kfree(file);
}

/* 创建路径下默认的文件夹 '.' 与 '..' */
int file_defaultdir (struct FsDevice *fsdev, char *path, 
        uint flag, uint mode)
{
    struct Inode *inode = NULL;
    char *p_path = NULL;

    /* 申请暂存路径的内存空间 */
    p_path = (char *)kalloc(kstrlen(path) + 4);
    if (p_path == NULL)
        return -1;

    /* 为传入的路径添加 '/' */
    kstrcpy(p_path, path);
    if (path[kstrlen(path)-1] != '/')
        kstrcat(p_path, "/");

    /* 创建目录项 '.' */
    inode = inode_getfs(fsdev, flag, mode);
    if(inode == NULL)
    {
        kfree(p_path);
        return -1;
    }
    kstrcat(p_path, ".");
    fsdev->fs->fsops->create(fsdev, inode, p_path);
    inode_put(inode);

    /* 创建目录项 '..' */
    inode = inode_getfs(fsdev, flag, mode);
    if (inode == NULL)
    {
        kfree(p_path);
        return -1;
    }
    kstrcat(p_path, ".");
    fsdev->fs->fsops->create(fsdev, inode, p_path);
    inode_put(inode);

    /* 释放暂存路径的内存 */
    kfree(p_path);
    return 0;
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
int file_open (struct File *file, char *path, uint flag, uint mode)
{
    int ret = 0;
    char *ap_path = NULL;   /* 绝对路径 */
    char *rp_path = NULL;   /* 相对路径 */
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
        {
            kfree(ap_path);
            return -1;
        }
        /* 去除路径中包含的文件系统的挂载路径  */
        rp_path = path_fsdev(fsdev, ap_path);

        /* 获取文件系统中该路径下所对应的目录项 */
        ditem = ditem_get(fsdev, rp_path);
        if (ditem != NULL)
        {
            inode = ditem->inode;
        }
        else
        {
            /* 判断是否要在实体文件系统中创建新的成员 */
            if ((flag & O_CREAT) != O_CREAT)
                return -1;
            
            /* 创建新的 inode 成员 */
            inode = inode_getfs(fsdev, flag, mode);
            if (inode == NULL)
                return -1;

            /* 在实体文件系统中创建对应成员 */
            ret = fsdev->fs->fsops->create(fsdev, inode, rp_path);
            if (ret < 0)
            {
                inode_put(inode);
                return -1;
            }

            /* 创建新的目录项 */
            ditem = ditem_create(fsdev, rp_path);
            if (ditem == NULL)
            {
                inode_put(inode);
                return -1;
            }
            ditem->inode = inode;

            /* 创建目录下的 '.' 与 '..' 对象 */
            if (flag & O_DIRECTORY)
            {
                file_defaultdir(fsdev, rp_path, flag, mode);
            }
        }
        kfree(rp_path);
        kfree(ap_path);
    }
    else /* 操作注册的设备 */
    {
        /* 跳过前缀 */
        while(*path == ':') 
            path++;

        /* 获取指定的设备 */
        dev = dev_get(path);
        if (dev == NULL)
            return -1;

        inode = inode_getdev(dev, flag, S_IRWXU);
        if (inode == NULL)
        {
            dev_put(dev);
            return -1;
        }
    }

    /* 初始化新打开的文件描述符 */
    file->inode = inode;
    file->ref  += 1;
    file->fops  = inode->fops;
    file->flags = flag;
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
    int ret = -1;

    if (file == NULL)
        return -1;
    if ((file->ref <= 0) || 
        (file->magic != FILE_MAGIC))
        return -1;

    if (file->fops->close != NULL)
        ret = file->fops->close(file);

    file->ref -= 1;
    file->inode->ref -= 1;

    /* 释放文件对象占用的资源 */
    if (file->inode->type == INODE_DEVICE)
    {
        dev_put(file->inode->dev);
        inode_put(file->inode);
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
int file_read (struct File *file, void *buf, uint len)
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
int file_write (struct File *file, void *buf, uint len)
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

/* 删除指定路径下的文件或目录 */
int file_unlink (char *path)
{
    int ret = 0;
    char *ap_path = NULL;
    struct DirItem *ditem = NULL;
    struct FsDevice *fsdev = NULL;

    if (path == NULL)
        return -1;

    /* 获取绝对路径 */
    ap_path = path_parser(NULL, path);
    if (ap_path == NULL)
        return -1;

    /* 获取所属的文件系统 */
    fsdev = fsdev_get(ap_path);
    if (fsdev == NULL)
    {
        kfree(ap_path);
        return -1;
    }

    /* 禁止删除文件系统挂载路径所对应的文件对象 */
    if (0 == kstrcmp(fsdev->path, ap_path))
    {
        if (ap_path[kstrlen(fsdev->path)] == '\0')
        {
            kfree(fsdev);
            kfree(ap_path);
            return -1;
        }
    }

    /* 获取该路径所对应的目录项 (确认该文件存在) */
    ditem = ditem_get(fsdev, ap_path);
    if (ditem == NULL)
    {
        kfree(fsdev);
        kfree(ap_path);
        return -1;
    }

    /* 按照不同的类型，处理不同的释放处理 */
    switch (ditem->inode->type)
    {
        case INODE_DEVICE:
            dev_put(ditem->inode->dev);
            break;
        case INODE_DIR:
        case INODE_FILE:
            /* 清除目录项与实体文件系统的关联 */
            if (ditem->fsdev->fs->fsops->unlink != NULL)
                ret = ditem->fsdev->fs->fsops->unlink(ditem);
            break;
        case INODE_PIPO:
            break;
        default: break;
    }
    inode_put(ditem->inode);
    ditem_put(ditem);
    ditem_destroy(ditem);
    fsdev_put(fsdev);
    kfree(ap_path);

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
int file_getdents(struct File *file, struct dirent *dirp, uint nbytes)
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
int file_lseek (struct File *file, uint offset, uint type)
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

/* 获取指定文件所属实体文件系统的信息 */
int file_fstatfs (struct File *file, struct statfs *buf)
{
    int ret = -1;

    if ((file == NULL) || (buf == NULL))
        return -1;
    if (file->magic != FILE_MAGIC)
        return -1;

    if (file->inode->fs->fsops->statfs != NULL)
    {
        ret = file->inode->fs->fsops->statfs(
                file->ditem->fsdev, buf);
    }
    return ret;
}

/* 获取文件对象的信息 */
int file_stat (struct File *file, struct stat *buf)
{
    int ret = -1;

    if ((file == NULL) || (buf == NULL))
        return -1;
    if (file->magic != FILE_MAGIC)
        return -1;
    
    if (file->fops->stat != NULL)
        ret = file->fops->stat(file, buf);

    return ret;
}

/* 修改文件路径所对应文件的名字 */
int file_rename (char *oldpath, char *newpath)
{
    int ret = -1;
    struct DirItem *oditem = NULL;
    struct DirItem *nditem = NULL;
    struct FsDevice *fsdev = NULL;
    char *ap_opath = NULL, *ap_npath = NULL;

    if ((oldpath == NULL) || (newpath == NULL))
        return -1;

    /* 格式化传入的路径 */
    ap_opath = path_parser(NULL, oldpath);
    ap_npath = path_parser(NULL, newpath);

    /* 获取旧路径所对应的文件系统 */
    fsdev = fsdev_get(ap_opath);
    if (fsdev == NULL)
        goto _exit_rename_path;

    /* 获取旧路径所对应的目录项 */
    oditem = ditem_get(fsdev, ap_opath);
    if (oditem == NULL)
        goto _exit_rename_path;

    /* 确认新目录项不存在 */
    nditem = ditem_get(fsdev, ap_npath);
    if (nditem != NULL)
        goto _exit_rename_ditem;

    /* 创建新路径的目录项 */
    nditem = ditem_create(fsdev, ap_npath);
    if (nditem == NULL)
        goto _exit_rename_ditem;

    /* 更新实体文件系统内的信息 */
    if (oditem->inode->fops->rename != NULL)
    {
        ret = oditem->inode->fops->rename(oditem, nditem);
    }

    /* 释放使用的资源 */
    ditem_destroy(nditem);

 _exit_rename_ditem:
    ditem_put(oditem);

 _exit_rename_path:
    kfree(ap_opath);
    kfree(ap_npath);

    return ret;
}
