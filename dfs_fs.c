/*
 * 存放 disk file system 对外的功能接口
 */
#include "defs.h"
#include "fcntl.h"
#include "dfs_priv.h"


/****************************************************
 *      dfs 文件系统文件操作接口
 ***************************************************/
int dfs_open (struct File *file)
{
    struct Inode *inode = NULL;
    struct dinode *node = NULL;

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
    struct disk_sb *sb = NULL;
    struct dinode *node = NULL;

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

    dnode_read(sb, node, buf, file->off, rLen);
    file->off += rLen;

    return rLen;
}
int dfs_write (struct File *file, void *buf, uint count)
{
    struct disk_sb *sb = NULL;
    struct dinode *node = NULL;

    if ((file == NULL) || (buf == NULL) || 
        (count == 0) || (file->inode == NULL))
        return -1;

    node = file->inode->data;
    if (node == NULL)
        return -1;

    sb = (struct disk_sb*)file->ditem->fsdev->data;
    if ((sb == NULL) || (sb->magic != DFS_MAGIC))
        return -1;

    /* 当写入的数据大于文件大小时，扩大该文件的大小 */
    count = dnode_write(sb, node, buf, file->off, count);

    file->off += count;

    return count;
}
int dfs_flush (struct File *file)
{
    struct disk_sb *sb = NULL;
    struct dinode *node = NULL;

    if (file == NULL)
        return -1;
    
    node = (struct dinode *)file->inode->data;
    if (node == NULL)
        return -1;
    
    sb = (struct disk_sb *)file->ditem->fsdev->data;
    if ((sb == NULL) || (sb->magic != DFS_MAGIC))
        return -1;
    
    dnode_flush(sb, node);

    return 0;
}
int dfs_lseek (struct File *file, uint offs, uint type)
{
    int ret = -1;
    struct dinode *node = NULL;

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
    struct disk_sb *sb = NULL;
    struct dinode *node = NULL;
    struct dinode *root = NULL;
    struct dinode *parent_node = NULL;

    if ((file == NULL) || (dirp == NULL) ||
        (file->inode == NULL))
        return -1;
    
    sb = dsb_read();
    if (sb == NULL)
        return -1;
    
    root = dnode_getroot(sb);
    if (root == NULL)
        return -1;
    
    disk_path_getlast(file->ditem->path, name);
    
    parent_node = dnode_find(sb, root, file->ditem->path, name);
    if (parent_node == NULL)
        return -1;

    /* 记录要读取的文件对象的数量 */
    num = count / sizeof(struct dirent);
    if (num == 0)
        return -1;
    end = file->off + num;

    /* 遍历 dinode 内的所有目录条目 */
	for(off = 0; off < parent_node->size; off += sizeof(dir))
	{
        dnode_read(sb, parent_node, (char*)&dir, off, sizeof(dir));

        if (dir.inum == 0)
            break;

        if (idx >= file->off)
        {
            node = dnode_alloc(sb, dir.inum);

            kstrcpy(dirp[cnt].name, dir.name);
            if (node->type == T_FILE)
                dirp[cnt].type = INODE_FILE;
            else if (node->type == T_DIR)
                dirp[cnt].type = INODE_DIR;
            dirp[cnt].namelen = kstrlen(dir.name);
            dirp[cnt].objsize = sizeof(struct dirent);
            dirp[cnt].datasize = node->size;

            dnode_free(sb, node);

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
 *      dfs 文件系统文件操作接口
 ***************************************************/
int dfs_mount (struct FsDevice *fsdev, uint flag, void *data)
{
    struct disk_sb *disksb = NULL;

    if ((fsdev == NULL) || (data == NULL))
        return -1;

    disksb = dsb_read();
    if (disksb == NULL)
        return -1;

    fsdev->data = disksb;

    return 0;
}
int dfs_unmount (struct FsDevice *fsdev)
{
    struct disk_sb *disksb = NULL;

    if (fsdev == NULL)
        return -1;
    disksb = (struct disk_sb *)fsdev->data;
    if (disksb == NULL)
        return -1;
    fsdev->data = NULL;

    dsb_write(disksb);

    return 0;
}
int dfs_statfs (struct FsDevice *fsdev, struct statfs *buf)
{
    struct disk_sb *disksb = NULL;

    if ((fsdev == NULL) || (buf == NULL))
        return -1;
    disksb = (struct disk_sb *)fsdev->data;
    if ((disksb == NULL) || (disksb->magic != DFS_MAGIC))
        return -1;

    kstrcpy(buf->name, "diskfs");

    buf->f_total = disksb->size;
    buf->f_bsize = 512;
    buf->f_block = (disksb->size + 511)/512;
    buf->f_bfree = 1;

    return 0;
}
int dfs_unlink (struct DirItem *ditem)
{
    struct disk_sb *sb = NULL;
    struct dinode *node = NULL;

    if ((ditem == NULL) ||
        (ditem->magic != DIRITEM_MAGIC))
        return -1;
    
    sb = (struct disk_sb *)ditem->fsdev->data;
    if (sb == NULL)
        return -1;

    /* 获取路径所对应的文件节点 */
    node = (struct dinode *)ditem->inode->data;
    if (node == NULL)
        return -1;

    dnode_put(sb, node);
    dnode_free(sb, node);
    
    return 0;
}
int dfs_stat (struct File *file, struct stat *buf)
{
    struct dinode *node = NULL;

    if ((file == NULL) || (buf == NULL))
        return -1;
    node = file->inode->data;
    if (node == NULL)
        return -1;
    
    buf->size = node->size;
    buf->name[0] = '\0';    /* TODO: */

    return 0;
}
int dfs_rename (struct DirItem *old_ditem, struct DirItem *new_ditem)
{
    char old_name[16], new_name[16];
    struct disk_sb *sb = NULL;
    struct dinode *root = NULL;
    struct disk_dirent *dir = NULL;
    struct dinode *parent_node = NULL;

    if ((old_ditem == NULL) || (new_ditem == NULL))
        return -1;
    
    sb = dsb_read();
    if ((sb == NULL) || (sb->magic != DFS_MAGIC))
        return -1;
    root = dnode_getroot(sb);
    if (root == NULL)
        return -1;

    disk_path_getlast(old_ditem->path, old_name);
    if (old_name[0] == '\0')
        return -1;
    disk_path_getlast(new_ditem->path, new_name);
    if (new_name[0] == '\0')
        return -1;

    parent_node = dnode_find(sb, root, old_ditem->path, old_name);

    dir = ddir_get(sb, parent_node, old_name);
    ddir_rename(sb, parent_node, dir, new_name);
    ddir_put(sb, parent_node, dir);

    return 0;
}
int dfs_lookup (struct FsDevice *fsdev, 
        struct Inode *inode, const char *path)
{
    char name[16];
    struct disk_sb *sb = NULL;
    struct dinode *node = NULL;
    struct dinode *root = NULL;
    struct dinode *parent_node = NULL;

    if ((fsdev == NULL) || (inode == NULL) || (path == NULL))
        return -1;

    sb = fsdev->data;
    if ((sb == NULL) || (sb->magic != DFS_MAGIC))
        return -1;

    root = dnode_getroot(sb);
    if (root == NULL)
        return -1;
    
    parent_node = dnode_find(sb, root, (char*)path, name);
    if (parent_node == NULL)
        return -1;
    
    node = ddir_read(sb, parent_node, name, 0);
    if (node == NULL)
    {
        dnode_free(sb, parent_node);
        return -1;
    }
    dnode_free(sb, parent_node);

    inode->data = node;
    inode->size = node->size;
    inode->flags = O_RDWR;
    inode->mode = S_IRWXU;

    if (node->type == T_DIR)
        inode->type = INODE_DIR;
    else
        inode->type = INODE_FILE;

    return 0;
}
int dfs_create (struct FsDevice *fsdev, struct Inode *inode, char *path)
{
    uint blknum;
    char name[16];
    struct disk_sb *sb = NULL;
    struct dinode *node = NULL;
    struct dinode *root = NULL;
    struct dinode *parent_node = NULL;

    if ((fsdev == NULL) || (inode == NULL) || (path == NULL))
        return -1;

    sb = fsdev->data;
    if ((sb == NULL) || (sb->magic != DFS_MAGIC))
        return -1;

    root = dnode_getroot(sb);
    if (root == NULL)
        return -1;
    
    parent_node = dnode_find(sb, root, path, name);
    if (parent_node == NULL)
        return -1;
    
    if (inode->type == INODE_FILE)
        blknum = dnode_get(sb, T_FILE);
    else
        blknum = dnode_get(sb, T_DIR);
    
    node = dnode_alloc(sb, blknum);
    ddir_write(sb, parent_node, name, blknum);

    inode->data = node;
    inode->size = 0;

    return 0;
}
int dfs_free (struct FsDevice *fsdev, struct Inode *inode)
{
    struct disk_sb *sb = NULL;
    struct dinode *node = NULL;

    if ((fsdev == NULL) || (inode == NULL))
        return -1;
    
    sb = dsb_read();
    if ((sb == NULL) || (sb->magic != DFS_MAGIC))
        return -1;

    node = (struct dinode *)inode->data;
    if (node == NULL)
        return -1;
    
    dnode_put(sb, node);
    dnode_free(sb, node);

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

    /* 初始化磁盘缓冲区模块 */
    init_iobuf();

    /* 将 dfs 注册为内核文件系统对象 */
    fsobj_register("diskfs", &dfs_fops, &dfs_fsops, TRUE);
}
