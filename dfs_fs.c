/*
 * 存放 disk file system 对外的功能接口
 */
#include "defs.h"
#include "fcntl.h"
#include "dfs_priv.h"


/****************************************************
 *      dfs 文件操作接口
 ***************************************************/
int dfs_open (struct File *file)
{
    struct Inode *inode = NULL;
    struct disk_inode *node = NULL;

    if ((file == NULL) || (file->magic != FILE_MAGIC))
        return -1;
    
    inode = file->inode;
    if (inode == NULL)
        return -1;

    node = inode->data;
    if (node == NULL)
        return -1;
    
    if (inode->flags & O_APPEND)
    {
        /* 在文件末尾继续写入 */
        file->off = node->size;
    }
    else
    {
        /* 丢弃该文件节点之前的内容 */
        if (inode->flags & O_TRUNC)
        {
            /* TODO */
            // kfree((void*)node->data);
            // node->data = NULL;
            node->size = 0;
        }
        file->off = 0;
    }

    /* 同步 inode 与 ramfs_node 的信息 */
    inode->size = node->size;

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
    uint rLen;
    struct disk_inode *node = NULL;

    if ((file == NULL) || (buf == NULL) || 
        (count == 0))
        return -1;

    node = file->inode->data;
    if (node == NULL)
        return -1;

    /* 确认文件实际可读写的长度 */
    if (count > (node->size - file->off))
        rLen = node->size - file->off;
    else
        rLen = count;

    dinode_read(node, buf, file->off, rLen);
    file->off += rLen;

    return rLen;
}
int dfs_write (struct File *file, void *buf, uint count)
{
    struct disk_inode *node = NULL;

    if ((file == NULL) || (buf == NULL) || 
        (count == 0) || (file->inode == NULL))
        return -1;

    node = file->inode->data;
    if (node == NULL)
        return -1;

    /* 当写入的数据大于文件大小时，扩大该文件的大小 */
    count = dinode_write(node, buf, file->off, count);

    file->off += count;

    return count;
}
int dfs_flush (struct File *file)
{
    struct disk_inode *node = NULL;

    if (file == NULL)
        return -1;

    /* 获取要操作的目标节点 */ 
    node = (struct disk_inode *)file->inode->data;
    if (node == NULL)
        return -1;

    /* 将磁盘节点的信息写回到磁盘 */
    dinode_flush(node);

    return 0;
}
int dfs_lseek (struct File *file, uint offs, uint type)
{
    int ret = -1;
    struct disk_inode *node = NULL;

    if ((file == NULL) || (file->inode == NULL))
        return -1;

    node = file->inode->data;
    if (node == NULL)
        return -1;

    switch (type)
    {
        case SEEK_SET:
            if (offs < file->inode->size)
                ret = file->off = offs;
            break;
        case SEEK_CUR:
            if ((file->off + offs) < (file->inode->size))
                ret = file->off + offs;
            break;
        case SEEK_END:
            ret = file->off = node->size;
            break;
        case SEEK_DATA:
            break;
        case SEEK_HOLE:
            break;
        default: break;
    }

    return ret;
}
int dfs_getdents (struct File *file, struct dirent *dirp, uint count)
{
    uint off;
    char name[16];
    uint num = 0, end = 0;
    uint cnt = 0, idx = 0;
    struct disk_dirent dir;
    struct disk_inode *node = NULL;
    struct disk_inode *parent = NULL;

    if ((file == NULL) || (dirp == NULL) ||
        (file->inode == NULL))
        return -1;

    /* 解析该路径中目标文件的名字，以及其父路径的磁盘节点 */
    parent = dinode_parent(file->ditem->path, name);
    if (parent == NULL)
        return -1;

    /* 记录要读取的文件对象的数量 */
    num = count / sizeof(struct dirent);
    if (num == 0)
        return -1;
    end = file->off + num;

    /* 遍历父节点 disk_inode 内的所有目录项 */
	for(off = 0; off < parent->size; off += sizeof(struct disk_dirent))
	{
        dinode_read(parent, (char *)&dir, off, sizeof(struct disk_dirent));

        /* 跳过被清空的成员 */
        if (dir.inum == 0)
            continue;

        /* 避免获取相同的成员 */
        if (idx >= file->off)
        {
            if (dir.inum != 0)
            {
                node = dinode_alloc(dir.inum);

                kstrcpy(dirp[cnt].name, dir.name);
                dirp[cnt].type = (node->type == DISK_DIR) ? INODE_DIR:INODE_FILE;
                dirp[cnt].namelen = kstrlen(dir.name);
                dirp[cnt].objsize = sizeof(struct dirent);
                dirp[cnt].datasize = node->size;

                dinode_free(node);
            }
            cnt += 1;
            file->off += 1;
        }
        /* 限制本次读取的数量 */
        if (++idx >= end)
            break;
    }

    /* 返回读取到的总内存大小 */
    return cnt * sizeof(struct dirent);
}


/****************************************************
 *      dfs 文件系统操作接口
 ***************************************************/
int dfs_mount (struct FsDevice *fsdev, uint flag, void *data)
{
    if (fsdev == NULL)
        return -1;
    fsdev->data = dsb_get();
    dinode_getroot();

    return 0;
}
int dfs_unmount (struct FsDevice *fsdev)
{
    if (fsdev == NULL)
        return -1;

    fsdev->data = NULL;
    dsb_put();

    return 0;
}
int dfs_statfs (struct FsDevice *fsdev, struct statfs *buf)
{
    struct disk_sb *sb = NULL;

    if ((fsdev == NULL) || (buf == NULL))
        return -1;
    sb = (struct disk_sb *)fsdev->data;
    if ((sb == NULL) || (sb->magic != DFS_MAGIC))
        return -1;

    kstrcpy(buf->name, "diskfs");

    buf->f_total = sb->size;
    buf->f_bsize = 512;
    buf->f_block = (sb->size + 511)/512;
    buf->f_bfree = 1;

    return 0;
}
/* 目前只能删除文件，而不能删除目录 */
int dfs_unlink (struct DirItem *ditem)
{
    char name[16];
    struct disk_inode *node = NULL;
    struct disk_inode *parent = NULL;
    struct disk_dirent *dir = NULL;

    if ((ditem == NULL) ||
        (ditem->magic != DIRITEM_MAGIC))
        return -1;

    /* 解析文件路径中的父节点，以及文件目标的名字 */
    parent = dinode_parent(ditem->path, name);
    if (parent == NULL)
        return -1;
    
    /* 从父节点中获取指定名字的目录项与磁盘节点 */
    dir = ddir_get(parent, name);
    node = dinode_alloc(dir->inum);

    if (node->type == DISK_FILE)
    {
        /* 释放单个文件 */
        ddir_release(parent, dir);

        /* 释放该文件占用的内存空间 */
        dinode_put(node);
        dinode_free(node);
    }
    else
    {
        /* TODO */
    }

    return 0;
}
int dfs_stat (struct File *file, struct stat *buf)
{
    struct disk_inode *node = NULL;

    if ((file == NULL) || (buf == NULL))
        return -1;
    
    /* 获取目标节点 */
    node = file->inode->data;
    if (node == NULL)
        return -1;

    /* 获取用户所需的节点信息 */
    buf->size = node->size;

    if (node->type == DISK_DIR)
        buf->type = VFS_DIR;
    else if(node->type == DISK_FILE)
        buf->type = VFS_FILE;

    kstrcpy(buf->fsname, file->inode->fs->name);
    disk_path_getlast(file->ditem->path, buf->name);

    return 0;
}
int dfs_rename (struct DirItem *old_ditem, struct DirItem *new_ditem)
{
    char old_name[16], new_name[16];
    struct disk_dirent *dir = NULL;
    struct disk_inode *parent_node = NULL;

    if ((old_ditem == NULL) || (new_ditem == NULL))
        return -1;

    disk_path_getlast(old_ditem->path, old_name);
    if (old_name[0] == '\0')
        return -1;
    disk_path_getlast(new_ditem->path, new_name);
    if (new_name[0] == '\0')
        return -1;

    parent_node = dinode_parent(old_ditem->path, old_name);

    /* 修改磁盘节点所对应的目录项中记录的节点名字 */
    dir = ddir_get(parent_node, old_name);
    ddir_rename(parent_node, dir, new_name);
    ddir_put(parent_node, dir);

    /* 修改虚拟文件系统中目录项记录的文件信息 */
    kstrcpy(old_ditem->path, new_ditem->path);

    return 0;
}
/* 通过文件路径查找对应的磁盘节点 */
int dfs_lookup (struct FsDevice *fsdev, 
        struct Inode *inode, const char *path)
{
    char name[16];
    struct disk_inode *node = NULL;
    struct disk_inode *parent = NULL;
    struct disk_dirent *dir = NULL;

    if ((fsdev == NULL) || (inode == NULL) || (path == NULL))
        return -1;

    /* 解析该路径中目标文件的名字，以及其父路径的磁盘节点 */
    parent = dinode_parent((char*)path, name);
    if (parent == NULL)
        return -1;

    /* 要解析的路径是否为文件系统的挂载路径 */
    if (name[0] != '\0')
    {
        /* 从父节点里获取指定名字的磁盘节点 */
        dir = ddir_get(parent, name);
        if (dir == NULL)
            return -1;
        node = dinode_alloc(dir->inum);
        ddir_put(parent, dir);

        dinode_free(parent);
        if (node == NULL)
            return -1;
    }
    else
    {
        node = parent;
    }

    /* 将磁盘节点与虚拟文件系统节点建立关联 */
    inode->flags = O_RDWR;
    inode->data = node;
    inode->size = node->size;
    inode->mode = S_IRWXU;
    inode->type = (node->type == DISK_DIR) ? INODE_DIR : INODE_FILE;

    return 0;
}
/* 创建新的磁盘索引节点 */
int dfs_create (struct FsDevice *fsdev, struct Inode *inode, char *path)
{
    uint inum;
    char name[16];
    struct disk_inode *node = NULL;
    struct disk_inode *parent = NULL;

    if ((fsdev == NULL) || (inode == NULL) || (path == NULL))
        return -1;

    /* 获取文件路径中的父节点与文件名 */
    parent = dinode_parent(path, name);
    if ((parent == NULL) || (name[0] == '\0'))
        return -1;
    if (parent->type != DISK_DIR)
        return -1;

    /* 从磁盘的索引节点块中获取一个空闲的对象 */
    if (inode->type == INODE_FILE)
        inum = dinode_get(DISK_FILE);
    else if (inode->type == INODE_DIR)
        inum = dinode_get(DISK_DIR);

    /* 在内存中创建与磁盘节点对应的内存节点 */
    node = dinode_alloc(inum);

    /* 在父节点中创建索引新节点的磁盘目录项 */
    if (-1 == ddir_create(parent, name, inum))
    {
        dinode_put(node);
        dinode_free(node);
    }

    /* 将新创建的磁盘节点与虚拟文件系统的节点建立联系 */
    inode->data = node;
    inode->size = 0;

    return 0;
}
/* 释放单个索引节点的内容 */
int dfs_free (struct FsDevice *fsdev, struct Inode *inode)
{
    struct disk_inode *node = NULL;

    if ((fsdev == NULL) || (inode == NULL))
        return -1;

    node = (struct disk_inode *)inode->data;
    if (node == NULL)
        return -1;
    
    dinode_put(node);
    dinode_free(node);

    return 0;
}


/****************************************************
 *      dfs 文件系统的结构体信息
 ***************************************************/
/* dfs 的文件操作接口 */
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
/* dfs 的文件系统操作接口 */
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

    /* 初始化磁盘缓冲区模块 */
    init_diskbuf();

    /* 将 dfs 注册为内核文件系统对象 */
    fsobj_register("diskfs", &dfs_fops, &dfs_fsops, FALSE);
}
