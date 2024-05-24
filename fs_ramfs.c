/*
 *          用 ram 来模拟的实体文件系统
 *
 * 1、允许在虚拟文件系统内挂载多个实体对象
 * 2、允许在上一个 ramfs 的文件路径中挂载新的 ramfs
 */
#include "fs_ramfs.h"
#include "defs.h"
#include "fcntl.h"


/****************************************************
 *      ramfs 文件系统仅内部使用的接口
 ***************************************************/
/* 申请一个 ramfs_node 对象 */
struct ramfs_node *_alloc_ramfs_node (void)
{
    struct ramfs_node *node;

    node = (struct ramfs_node *)kalloc(sizeof(struct ramfs_node));
    if (node == NULL)
        return NULL;

    list_init(&node->siblist);
    list_init(&node->sublist);

    return node;
}

/* 释放已申请的 ramfs_node 对象 */
void _free_ramfs_node (struct FsDevice *fsdev, struct ramfs_node *node)
{
    struct ramfs_sb *sb;

    if ((fsdev == NULL) || (node == NULL))
        return;

    sb = fsdev->data;
    if ((sb == NULL) || (sb->magic != RAMFS_MAGIC))
        return;

    kDISABLE_INTERRUPT();
    list_del(&node->siblist);
    kENABLE_INTERRUPT();

    if (node->data != NULL)
        kfree(node->data);

    sb->size -= sizeof(struct ramfs_node) + node->size;

    kfree(node);
}

/* 释放目录节点下的所有子节点 */
int _free_sublist (struct FsDevice *fsdev, struct ramfs_node *node)
{
    ListEntry_t *list;
    struct ramfs_node *sub_node;

    if (node == NULL)
        return - 1;

    /* 遍历当前节点下的所有子节点 */
    list_for_each(list, &node->sublist)
    {
        sub_node = list_container_of(list, struct ramfs_node, siblist);

        /* 递归所有的文件夹节点 */
        if (sub_node->type == RAMFS_DIR)
            _free_sublist(fsdev, sub_node);

        _free_ramfs_node(fsdev, sub_node);
    }

    return 0;
}

/* 解析路径中第一个节点的名字, 并返回剩下的路径内容
 *
 * path: 要解析的路径
 * name：存放第一个节点字符的缓冲区
 */
static char *ramfs_fstr_getfirst (char *path, char *name)
{
    return fstr_getfirst(path, name);
}

/* 解析路径中最后一个节点的名字，并返回该节点前的父节点路径
 *
 * path: 要解析的路径
 * parentPath: 存放父节点路径的缓冲区
 * name：最后一个节点的名字
 *
 * 返回值：-1为失败
 */
static int ramfs_fstr_getlast (char *path, char *parentPath, char *name)
{
    return fstr_getlast(path, parentPath, name);
}

/* 获取文件路径所对应的节点 ( 传入的必须是绝对路径 ) */
struct ramfs_node *_path_getnode (struct ramfs_sb *sb, char *path)
{
    ListEntry_t *list;
    struct ramfs_node *cur_node, *sub_node;
    char first_name[RAMFS_NAME_LEN], *cur_path;

    if ((sb == NULL) || (path == NULL))
        return NULL;

    cur_path = path;
    cur_node = &sb->root;

 _loop_path_parser:
    /* 读取到了最后一个文件 */
    if (*cur_path == '\0')
        return cur_node;

    /* 确保路径上的都是文件夹 */
    if (cur_node->type != RAMFS_DIR)
        return NULL;

    /* 获取当前 cur_path 路径下的第一个文件对象的名字 */
    cur_path = ramfs_fstr_getfirst(cur_path, first_name);
    if (first_name[0] == '\0')
        return NULL;    // 避免传入的路径仅为斜杠的情况

    /* 遍历目录节点下的所有子对象 */
    list_for_each(list, &cur_node->sublist)
    {
        sub_node = list_container_of(list, struct ramfs_node, siblist);

        /* 确认是否为要查找的下一个子对象 */
        if (0 == kstrcmp(sub_node->name, first_name))
        {
            cur_node = sub_node;
            goto _loop_path_parser;
        }
    }

    return NULL;
}


/****************************************************
 *      ramfs 文件系统文件操作接口
 ***************************************************/
/* 开启指定的文件，使其能被文件系统的接口所操作 */
int ramfs_open (struct Inode *inode)
{
    struct ramfs_node *node;

    if ((inode == NULL) || (inode->magic != INODE_MAGIC))
        return -1;

    /* 在调用该函数前，虚拟文件系统应调用 lookup 接口，
     * 查找文件路径所对应的 inode 以及其对应的 node
     */
    node = inode->data;
    if (node == NULL)
        return -1;

    if (inode->flags & O_APPEND)
    {
        /* 在文件末尾继续写入 */
        inode->writeoff = node->size;
    }
    else
    {
        /* 丢弃该文件节点之前的内容 */
        if (inode->flags & O_TRUNC)
        {
            kfree((void*)node->data);
            node->data = NULL;
            node->size = 0;
        }
        inode->writeoff = 0;
    }

    /* 同步 inode 与 ramfs_node 的信息 */
    inode->size = node->size;
    inode->readoff = 0;

    return 0;
}
/* 关闭已打开的文件 */
int ramfs_close (struct Inode *inode)
{
    /* 不做任何操作，因为文件需要继续保存与文件系统中 */
    return 0;
}
/* 用于设置指定文件的功能 */
int ramfs_ioctl (struct Inode *inode, int cmd, void *args)
{
    return 0;
}
/* 将缓冲区数据从指定文件中读取 */
int ramfs_read (struct Inode *inode, void *buf, unsigned int count)
{
    unsigned int rLen;
    struct ramfs_node *node;

    if ((inode == NULL) || (buf == NULL) || (count == 0))
        return -1;

    node = inode->data;
    if ((node == NULL) || (node->data != NULL))
        return -1;

    /* 确认文件实际可读写的长度 */
    if (count > (inode->size - inode->readoff))
        rLen = inode->size - inode->readoff;
    else
        rLen = count;

    kmemcpy(buf, node->data, rLen);
    inode->readoff += rLen;

    return rLen;
}
/* 将缓冲区数据写入指定文件中 */
int ramfs_write (struct Inode *inode, void *buf, unsigned int count)
{
    char *memAddr;
    struct ramfs_sb *sb;
    struct ramfs_node *node;

    if ((inode == NULL) || (buf == NULL) || (count == 0))
        return -1;

    if ((node = inode->data) == NULL)
        return -1;

    sb = ((struct ramfs_node*)inode->data)->sb;
    if ((sb == NULL) || (sb->magic != RAMFS_MAGIC))
        return -1;

    /* 当写入的数据大于文件大小时，扩大该文件的大小 */
    if (count > (node->size - inode->writeoff))
    {
        /* 不做精确计算，直接扩张出足够大的空间 */
        memAddr = (char *)kalloc(node->size + count);
        kmemmove(memAddr, node->data, node->size);

        /* 递增超级块的记录 */
        sb->size += count;

        node->data = memAddr;
        node->size += count;

        inode->size = node->size;
    }
    kmemcpy(&node->data[inode->writeoff], buf, count);

    inode->writeoff += count;

    return count;
}
/* 将文件系统缓存的数据写入磁盘 */
int ramfs_flush (struct Inode *inode)
{
    return 0;
}
/* 修改文件节点内的偏移值 */
int ramfs_lseek (struct Inode *inode, bool type, unsigned int offs)
{
    struct ramfs_node *node;

    node = inode->data;
    if ((node == NULL) || (node->data != NULL))
        return -1;

    if (type)
    {
        if (offs <= (node->size - inode->writeoff))
            inode->writeoff += offs;

        return inode->writeoff;
    }

    if (offs <= (node->size - inode->readoff))
            inode->readoff += offs;

    return inode->readoff;
}
/* 获取目录节点下指定数量的子节点信息 */
int ramfs_getdents (struct Inode *inode, struct dirent *dirp, unsigned int count)
{
    ListEntry_t *list;
    unsigned int num = 0, end = 0, idx = 0;
    struct ramfs_node *node, *next_node;

    if ((inode == NULL) || (dirp == NULL))
        return -1;

    /* 获取 inode 对应的 ramfs 的 node 节点 */
    if ((node = inode->data) == NULL)
        return -1;

    /* 记录要读取的文件对象的数量 */
    num = count / sizeof(struct dirent);
    if (num == 0)
        return -1;
    end = inode->readoff + num;

    /* 遍历目录下的所有文件对象 */
    list_for_each(list, &node->sublist)
    {
        next_node = list_container_of(list, struct ramfs_node, siblist);

        /* 确认是未读取过的文件对象 */
        if (idx >= inode->readoff)
        {
            /* 添加要读取的信息 */
            kstrcpy(dirp[num].name, next_node->name);
            if (next_node->type == RAMFS_FILE)
                dirp[num].type = INODE_FILE;
            else if (next_node->type == RAMFS_DIR)
                dirp[num].type = INODE_DIR;
            dirp[num].namelen = kstrlen(next_node->name);
            dirp[num].objsize = sizeof(struct dirent);

            num += 1;
            inode->readoff += 1;
        }

        /* 限制本次读取的数量 */
        if (++idx > end)
            break;
    }

    return num * sizeof(struct dirent);
}


/****************************************************
 *      ramfs 文件系统文件操作接口
 ***************************************************/
/* 创建文件系统的超级块，并初始化 */
int ramfs_mount (struct FsDevice *fsdev, unsigned long flag, void *data)
{
    struct ramfs_sb *sb;

    /* 创建当前文件系统的超级块 */
    sb = (struct ramfs_sb *)kalloc(sizeof(struct ramfs_sb));
    if (sb == NULL)
        return -1;

    /* 初始化当前文件系统的超级块 */
    sb->magic = RAMFS_MAGIC;
    sb->size  = sizeof(struct ramfs_sb);
    sb->flag  = flag;

    sb->root.type = RAMFS_DIR;
    sb->root.name[0] = '/';
    sb->root.sb = sb;

    list_init(&sb->siblist);
    list_init(&sb->root.siblist);
    list_init(&sb->root.sublist);

    /* 将超级块与文件系统结构体建立联系 */
    fsdev->data = sb;

    return 0;
}
/* 注销文件系统创建的超级块，并释放拥有的所有子节点 */
int ramfs_unmount (struct FsDevice *fsdev)
{
    struct ramfs_sb *sb;

    if (fsdev == NULL)
        return -1;
    sb = fsdev->data;
    if ((sb == NULL) || (sb->magic != RAMFS_MAGIC))
        return -1;

    /* 破坏魔幻数，防止在卸载过程中被调用 */
    sb->magic = 0;

    /* 释放根目录链表下的所有子节点 */
    _free_sublist(fsdev, &sb->root);
    /* 释放超级块本身占用的内容 */
    kfree(sb);

    /* 取消 inode 与文件系统的关联 */
    fsdev->data = NULL;

    return 0;
}
/* 获取文件系统的信息 */
int ramfs_statfs (struct FsDevice *fsdev, struct statfs *buf)
{
    struct ramfs_sb *sb;

    if ((fsdev == NULL) || (buf == NULL))
        return -1;
    sb = fsdev->data;
    if ((sb == NULL) || (sb->magic != RAMFS_MAGIC))
        return -1;

    /* 写入文件系统当前的信息 */
    buf->f_bsize  = 512;
    buf->f_blocks = (sb->size + 511)/512;
    buf->f_bfree  = 1;

    return 0;
}
/* 删除指定的文件或目录 */
int ramfs_unlink (struct FsDevice *fsdev, char *path)
{
    struct ramfs_sb *sb;
    struct ramfs_node *node;

    if ((fsdev == NULL) || (path == NULL))
        return -1;
    sb = fsdev->data;
    if ((sb == NULL) || (sb->magic != RAMFS_MAGIC))
        return -1;

    /* 获取路径所对应的文件节点 */
    node = _path_getnode(sb, path);
    if (node == NULL)
        return -1;

    /* 从文件系统中移除该节点 */
    kDISABLE_INTERRUPT();
    list_del (&node->siblist);
    kENABLE_INTERRUPT();

    /* 释放节点链表中的所有子节点 */
    _free_sublist(fsdev, node);
    /* 释放节点本身占用的内存空间 */
    _free_ramfs_node(fsdev, node);

    return 0;
}
/* 获取指定文件或目录的信息  */
int ramfs_stat (struct FsDevice *fsdev, char *path, struct stat *buf)
{
    struct ramfs_sb     *sb;
    struct ramfs_node   *node;

    if ((fsdev == NULL) || (path == NULL) || (buf == NULL))
        return -1;
    sb = fsdev->data;
    if ((sb == NULL) || (sb->magic != RAMFS_MAGIC))
        return -1;

    /* 获取文件路径所对应的节点 */
    node = _path_getnode(sb, path);
    if (node == NULL)
        return -1;

    /* 将节点信息写入缓冲区 */
    buf->size = node->size;

    return 0;
}
/* 修改指定文件对象的名字 */
int ramfs_rename (struct FsDevice *fsdev, char *oldpath, char *newpath)
{
    struct ramfs_sb *sb;
    struct ramfs_node *node;
    char parent_path[RAMFS_PATH_MAT], new_name[RAMFS_NAME_LEN];

    if ((fsdev == NULL) || (oldpath == NULL) || (newpath == NULL))
        return -1;
    sb = fsdev->data;
    if ((sb == NULL) || (sb->magic != RAMFS_MAGIC))
        return -1;

    /* 排除创建新的同名文件 */
    if (NULL != _path_getnode(sb, newpath))
        return -1;

    /* 获取旧文件路径的节点 */
    node = _path_getnode(sb, oldpath);
    if (node == NULL)
        return -1;

    /* 获取新文件的名字 */
    kmemset(new_name, 0, RAMFS_NAME_LEN);
    if (-1 == ramfs_fstr_getlast (newpath, parent_path, new_name))
        return -1;

    /* 修改名字 */
    kDISABLE_INTERRUPT();
    kstrcpy(node->name, new_name);
    kENABLE_INTERRUPT();

    return 0;
}
/* 查找指定路径下的 ramfs_node 对象，传入的 path 必须是绝对路径 */
int ramfs_lookup (struct FsDevice *fsdev, struct Inode *inode, char *path)
{
    struct ramfs_sb     *sb;
    struct ramfs_node   *node;

    if ((fsdev == NULL) || (inode == NULL) || (path == NULL))
        return -1;
    sb = fsdev->data;
    if ((sb == NULL) || (sb->magic != RAMFS_MAGIC))
        return -1;

    /* 获取路径对应的 ramfs_node 节点 */
    node = _path_getnode(sb, path);
    if (node == NULL)
        return -1;

    /* 初始化 inode 的内容 */
    if (node->type == RAMFS_DIR)
        inode->type = INODE_DIR;
    else
        inode->type = INODE_FILE;
    inode->fs = fsdev->fs;
    inode->data = node; /* 非常重要的一步操作 */
    inode->readoff = 0;
    inode->writeoff = 0;
    inode->size = node->size;
    inode->flags = node->flags;

    return 0;
}
/* 创建文件路径下与 inode 对应的 ramfs_node */
int ramfs_create (struct FsDevice *fsdev, struct Inode *inode, char *path)
{
    struct ramfs_sb *sb;
    struct ramfs_node *node, *parent_node;
    char parent_path[RAMFS_PATH_MAT], node_name[RAMFS_NAME_LEN];

    if ((fsdev == NULL) || (inode == NULL) || (path == NULL))
        return -1;

    sb = fsdev->data;
    if ((sb == NULL) || (sb->magic != RAMFS_MAGIC))
        return -1;

    /* 创建要加入文件系统的 ramfs_node 对象 */
    node = _alloc_ramfs_node();
    if (node == NULL)
        return -1;

    /* 获取父节点的 ramfs_node 和要创建的节点名字 */
    kmemset(parent_path, 0, RAMFS_PATH_MAT);
    kmemset(node_name, 0, RAMFS_NAME_LEN);
    if (-1 == ramfs_fstr_getlast (path, parent_path, node_name))
        return -1;
    if (NULL == (parent_node = _path_getnode(sb, parent_path)))
        return -1;

    /* 递增当前超级块记录的内存空间大小 */
    sb->size += sizeof(struct ramfs_node);

    /* 初始化新创建的节点 */
    if (inode->type == INODE_FILE)
        node->type = RAMFS_FILE;
    else
        node->type = RAMFS_DIR;
    kstrcpy(node->name, node_name);
    node->sb = sb;

    /* 让 inode 与 ramfs_node 建立联系 */
    inode->data = node;

    /* 将创建的节点添加到其父节点的链表中 */
    kDISABLE_INTERRUPT();
    list_add_after(&parent_node->sublist, &node->siblist);
    kENABLE_INTERRUPT();

    return 0;
}
/* 释放与 inode 对应 ramfs_node 的链接 */
int ramfs_free (struct FsDevice *fsdev, struct Inode *inode)
{
    struct ramfs_sb *sb;

    if ((fsdev == NULL) || (inode == NULL))
        return -1;

    sb = fsdev->data;
    if ((sb == NULL) || (sb->magic != RAMFS_MAGIC))
        return -1;

    /* ramfs_node 会继续存在于文件系统中 */
    inode->data = NULL;

    return 0;
}


/****************************************************
 *      ramfs 文件系统的结构体信息
 ***************************************************/
/* ramfs 文件系统的文件操作接口 */
struct FileOperation ramfs_fops =
{
    .open     = ramfs_open,
    .close    = ramfs_close,
    .ioctl    = ramfs_ioctl,
    .read     = ramfs_read,
    .write    = ramfs_write,
    .flush    = ramfs_flush,
    .lseek    = ramfs_lseek,
    .getdents = ramfs_getdents,
};
/* ramfs 文件系统的系统操作接口 */
struct FileSystemOps ramfs_fsops =
{
    .mount   = ramfs_mount,
    .unmount = ramfs_unmount,
    .statfs  = ramfs_statfs,
    .unlink  = ramfs_unlink,
    .stat    = ramfs_stat,
    .rename  = ramfs_rename,
    .lookup  = ramfs_lookup,
    .create  = ramfs_create,
    .free    = ramfs_free,
};


/****************************************************
 *      ramfs 文件系统对外的接口
 ***************************************************/
/* 初始化当前文件系统, 并注册到系统内核 */
void dfs_ramfs_init (void)
{
    fsdev_register("ramfs", &ramfs_fops, &ramfs_fsops, TRUE);
}

