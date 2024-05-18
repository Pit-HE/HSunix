/*
 * 用 ram 来模拟的实体文件系统
 * ( 该文件系统是用于测试虚拟文件系统是否正常工作 )
 * ( 该文件系统是可以在单个系统内挂载多个的 )
 */
#include "dfs_ramfs.h"
#include "defs.h"
#include "fcntl.h"

/* 路径分离，解析传文件路径，分离其中的父节点路径与文件名
 *
 * 返回值：-1为失败
 */
static int _path_separate (char *path, char *parentPath, char *fileName)
{
    return 0;
}
/* 解析路径中的第一个节点的名字
 *
 * 返回值：-1为失败
 */
static int _path_getfirst (char *path, char *name)
{
    char *ptr = path;

    while(*ptr == '/')
        ptr += 1;

    while(*ptr != '/' && *ptr)
        *name++ = *ptr++;
    *name = '\0';

    return 0;
}

/* 获取文件路径中所对应的文件节点 */
static struct ramfs_node *_path_parser (struct ramfs_sb *sb, char *path)
{
    char subdir_name[RAMFS_NAME_LEN];

    if (-1 == _path_getfirst(path, subdir_name))
        return NULL;

    /* TODO: 完善文件路径解析的功能 */

    return NULL;
}

/* 获取文件路径中所对应文件节点的父节点 */
static struct ramfs_node *_path_parent (struct ramfs_sb *sb, char *path)
{
    char parentPath[RAMFS_PATH_MAT];

    if (path == NULL)
        return NULL;
    if (-1 == _path_separate(path, parentPath, NULL))
        return NULL;

    return _path_parser(sb, parentPath);
}

/* 释放目录节点下的所有节点 ( 需要用到递归 ) */
static int _free_sublist (struct ramfs_node *dir)
{
    if (dir->type != RAMFS_DIR)
        return -1;

    return 0;
}

/****************************************************
 *      ramfs 文件系统文件操作接口
 */
int ramfs_open (struct Inode *inode)
{
    struct ramfs_node *node;

    if ((inode == NULL) || (inode->magic != INODE_MAGIC))
        return -1;

    /* 丢弃该文件节点之前的内容 */
    if (inode->flags & O_TRUNC)
    {
        inode->size = 0;
        inode->offs = 0;

        kfree (node->data);
        node->data = NULL;
        node->size = 0;
    }
    node->offs = 0;
    inode->offs = 0;

    return 0;
}
int ramfs_close (struct Inode *inode)
{
    return 0;
}
int ramfs_ioctl (struct Inode *inode, int cmd, void *args)
{
    return 0;
}
int ramfs_read (struct Inode *inode, void *buf, unsigned int count)
{
    return 0;
}
int ramfs_write (struct Inode *inode, void *buf, unsigned int count)
{
    return 0;
}
int ramfs_lseek (struct Inode *inode, unsigned int offset)
{
    return 0;
}
int ramfs_getdents (struct Inode *inode, struct Dirent *dirp, unsigned int count)
{
    return 0;
}

/****************************************************
 *      ramfs 文件系统文件操作接口
 */
/* 创建文件系统的超级块，并初始化 */
int ramfs_mount (struct FileSystem *fs, unsigned long flag, void *data)
{
    struct ramfs_sb *sb;

    sb = (struct ramfs_sb *)kalloc(sizeof(struct ramfs_sb));
    if (sb == NULL)
        return -1;
    kmemset(sb, 0, sizeof(struct ramfs_sb));

    sb->magic = RAMFS_MAGIC;
    sb->size  = sizeof(struct ramfs_sb);

    sb->root.type = RAMFS_DIR;
    sb->root.name[0] = '/';
    sb->root.sb = sb;

    list_init(&sb->siblist);
    list_init(&sb->root.siblist);
    list_init(&sb->root.sublist);

    fs->data = sb;

    return 0;
}
/* 注销文件系统创建的超级块，并释放拥有的所有子节点 */
int ramfs_unmount (struct FileSystem *fs)
{
    struct ramfs_sb *sb;

    if (fs == NULL)
        return -1;
    sb = fs->data;
    if (sb == NULL)
        return -1;

    _free_sublist(&sb->root);
    kfree(sb);

    fs->data = NULL;

    return 0;
}
/* 获取文件系统的信息 */
int ramfs_statfs (struct FileSystem *fs, struct statfs *buf)
{
    struct ramfs_sb *sb;

    if ((fs == NULL) || (buf == NULL))
        return -1;
    sb = fs->data;
    if (sb == NULL)
        return -1;

    buf->f_bsize  = 512;
    buf->f_blocks = (sb->size + 511)/512;
    buf->f_bfree  = 1;

    return 0;
}
/* 删除指定的文件或目录 */
int ramfs_unlink (struct FileSystem *fs, char *path)
{
    struct ramfs_sb     *sb;
    struct ramfs_node   *node;

    if ((fs == NULL) || (path))
        return -1;
    sb = fs->data;
    if (sb == NULL)
        return -1;

    node = _path_parser(sb, path);
    if (node == NULL)
        return -1;

    kDISABLE_INTERRUPT();
    list_del (&node->siblist);
    kENABLE_INTERRUPT();

    _free_sublist(node);

    if (node->data != NULL)
        kfree(node->data);
    kfree(node);

    return 0;
}
/* 获取指定文件或目录的信息  */
int ramfs_stat (struct FileSystem *fs, char *path, struct stat *buf)
{
    struct ramfs_sb     *sb;
    struct ramfs_node   *node;

    if ((fs == NULL) || (path))
        return -1;
    sb = fs->data;
    if (sb == NULL)
        return -1;

    node = _path_parser(sb, path);
    if (node == NULL)
        return -1;

    buf->size = node->size;

    return 0;
}
int ramfs_rename (struct FileSystem *fs, char *oldpath, char *newpath)
{
    struct ramfs_sb     *sb;
    struct ramfs_node   *node, *parent_node;
    char   parent_path[RAMFS_PATH_MAT], new_name[RAMFS_NAME_LEN];

    if ((fs == NULL) || (oldpath))
        return -1;
    sb = fs->data;
    if (sb == NULL)
        return -1;

    /* 排除创建新的同名文件 */
    node = _path_parser(sb, newpath);
    if (node != NULL)
        return -1;

    /* 获取旧文件路径的节点 */
    node = _path_parser(sb, oldpath);
    if (node == NULL)
        return -1;

    /* 获取新文件的信息 */
    kmemset(new_name, 0, RAMFS_NAME_LEN);
    _path_separate (newpath, parent_path, new_name);
    if (new_name[0] == '\0')
        return -1;

    /* 获取新文件路径的父节点 */
    parent_node = _path_parent(sb, newpath);
    if(parent_node == NULL)
        return -1;

    /* 从同级的链表里移除 */
    kDISABLE_INTERRUPT();
    list_del_init(&node->siblist);
    kENABLE_INTERRUPT();

    /* 修改名字 */
    kstrcpy(node->name, new_name);

    /* 将节点添加回父节点之后 */
    kDISABLE_INTERRUPT();
    list_add_after(&parent_node->sublist, &node->siblist);
    kENABLE_INTERRUPT();

    return 0;
}
/* 通过传入的文件路径，解析文件的 inode 对象 */
int ramfs_lookup (struct FileSystem *fs, char *path, struct Inode *inode)
{
    struct ramfs_sb     *sb;
    struct ramfs_node   *node;

    if ((fs == NULL) || (path))
        return -1;
    sb = fs->data;
    if (sb == NULL)
        return -1;

    node = _path_parser(sb, path);
    if (node == NULL)
        return -1;

    if (node->type == RAMFS_DIR)
        inode->type = I_DIR;
    else
        inode->type = I_FILE;

    inode->size = node->size;
    inode->data = node;

    return 0;
}

/****************************************************
 *      ramfs 文件系统的结构体信息
 */
/* ramfs 文件系统的文件操作接口 */
struct FileOperation ramfs_fops =
{
    .open   = ramfs_open,
    .close  = ramfs_close,
    .ioctl  = ramfs_ioctl,
    .read   = ramfs_read,
    .write  = ramfs_write,
    .lseek  = ramfs_lseek,
    .getdents = ramfs_getdents,
};
/* ramfs 文件系统的系统操作接口 */
struct FileSystemOps ramfs_fsops =
{
    .mount      = ramfs_mount,
    .unmount    = ramfs_unmount,
    .statfs     = ramfs_statfs,
    .unlink     = ramfs_unlink,
    .stat       = ramfs_stat,
    .rename     = ramfs_rename,
};
/* 完整的 ramfs 文件系统对象 */
struct FileSystem ramfs =
{
    "ramfs",
    &ramfs_fops,
    &ramfs_fsops,
};



/* ramfs 文件系统对外的接口，初始化当前文件系统 */
void dfs_ramfs_init (void)
{
    fsdev_register(&ramfs);
}

