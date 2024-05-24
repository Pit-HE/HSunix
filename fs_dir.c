/*
 *  管理并操作文件系统的目录项
 */
#include "defs.h"
#include "file.h"



/* 定义存放目录项的哈希数组 */
#define DIR_HASH_BUFF_SIZE      32
ListEntry_t ditem_hashlist[DIR_HASH_BUFF_SIZE];



/* 计算目录项所对应的哈希值 */
static int ditem_hash(struct FsDevice *fsdev, char *path)
{
    int hash = 0;

    if ((fsdev == NULL) || (path == NULL))
        return -1;

    while(*path != '\0')
        hash += *path++;

    hash ^= *(int*)fsdev;
    hash &= DIR_HASH_BUFF_SIZE - 1;

    return hash;
}

/* 目录项路径格式化，去除路径中可能包含的文件系统挂路径 */
static char *ditem_pathformat(
        struct FsDevice *fsdev, char *path)
{
    unsigned int len;

    len = kstrlen(fsdev->path);
    if (len == 0)
        return NULL;

    if (0 == kstrncmp(fsdev->path, path, len))
        path += len;

    return path;
}

/* 查找指定的目录项 */
static struct DirItem *ditem_find (
        struct FsDevice *fsdev, char *path)
{
    int index;
    ListEntry_t *list;
    struct DirItem *ditem;

    if ((fsdev == NULL) || (path == NULL))
        return NULL;

    /* 去除字符串中包含的文件系统挂载路径 */
    path = ditem_pathformat(fsdev, path);
    if (path == NULL)
       *path = '/';

    /* 获取所寻目录项的哈希值 */
    index = ditem_hash(fsdev, path);

    /* 遍历链表，获取指定的目录项 */
    list_for_each(list, &ditem_hashlist[index])
    {
        ditem = list_container_of(list, struct DirItem, list);
        if ((ditem->fsdev == fsdev) &&
            (0 == kstrcmp(ditem->path, path)))
        {
            return ditem;
        }
    }
    return NULL;
}

/* 创建新的目录项，并初始化 */
struct DirItem *ditem_alloc (
        struct FsDevice *fsdev, char *path)
{
    struct DirItem *ditem;

    if ((fsdev == NULL) || (path == NULL))
        return NULL;

    ditem = (struct DirItem *)kalloc(sizeof(struct DirItem));
    if (ditem == NULL)
        return NULL;

    /* 跳过 path 中包含的关于文件系统挂载的路径 */
    path = ditem_pathformat(fsdev, path);

    ditem->path = kstrdup(path);
    if (ditem->path == NULL)
    {
        kfree (ditem);
        return NULL;
    }

    /* 初始化目录项的成员 */
    ditem->magic = DIRITEM_MAGIC;
    ditem->state |= DITEM_ALLOC;
    ditem->ref = 0;
    ditem->fsdev = fsdev;
    list_init(&ditem->list);

    return NULL;
}

/* 释放已经申请的目录项的内存空间 */
int ditem_free (struct DirItem *dir)
{
    if (dir == NULL)
        return -1;
    if ((dir->magic != DIRITEM_MAGIC) &&
        (dir->ref == 0))
    dir->magic = 0;

    list_del(&dir->list);

    kfree(dir->path);
    kfree(dir);

    return 0;
}

/* 将目录项添加到模块的哈希数组中 */
void ditem_add (struct DirItem *ditem)
{
    int hash;

    if (ditem == NULL)
        return;
    if (ditem->magic != DIRITEM_MAGIC)
        return;

    hash = ditem_hash(ditem->fsdev, ditem->path);

    kDISABLE_INTERRUPT();
    list_add_before(&ditem_hashlist[hash], &ditem->list);
    ditem->state |= DITEM_HASH;
    kENABLE_INTERRUPT();
}

/* 获取指定路径下的目录项, 若不存在则创建它 */
struct DirItem *ditem_get (struct FsDevice *fsdev,
        char *path, uint flag)
{
    char *ditem_path;
    struct Inode *inode;
    struct DirItem *ditem;

    if ((fsdev == NULL) || (path == NULL))
        return NULL;

    /* 去除 path 中可能包含的文件系统挂载的路径 */
    ditem_path = ditem_pathformat(fsdev, path);
    if (*ditem_path == '\0')
        *ditem_path = '/';

    /* 确认目录项是否已经存在 */
    ditem = ditem_find(fsdev, ditem_path);
    if (ditem != NULL)
    {
        ditem->ref += 1;
        return ditem;
    }

    /* 确认实体文件系统是可查找的 */
    if (fsdev->fs->fsops->lookup == NULL)
        return NULL;

    /* 申请新的目录项 */
    ditem = ditem_alloc(fsdev, ditem_path);
    if (ditem == NULL)
        return NULL;

    /* 处理该目录项还未存在的情况 */
    inode = inode_alloc();
    if (inode == NULL)
    {
        ditem_free(ditem);
        return NULL;
    }
    inode_init(inode, flag, fsdev->fs->fops, 0);

    /* 查找与路径匹配的 inode */
    if (-1 == fsdev->fs->fsops->lookup(fsdev, inode, path))
    {
        ditem_free(ditem);
        inode_free(inode);
        return NULL;
    }
    inode->ref += 1;

    /* 将目录项与 inode 建立联系 */
    ditem->inode = inode;
    ditem->ref += 1;
    ditem_add(ditem);

    return ditem;
}

/*      释放已经获取的目录项
 * (目录项不会删除，需要继续保留在内存中)
 */
void ditem_put (struct DirItem *ditem)
{
    if (ditem == NULL)
        return;
    if (ditem->magic != DIRITEM_MAGIC)
        return;
    if (ditem->ref == 0)
        return;

    ditem->ref -= 1;
    ditem->state = DITEM_CLOSE;
}

/* 获取目录项的绝对路径 */
char *ditem_path (struct DirItem *ditem)
{
    char *path;
    int fsdev_len, ditem_len;


    if (ditem == NULL)
        return NULL;
    if (ditem->magic != DIRITEM_MAGIC)
        return NULL;

    fsdev_len = kstrlen(ditem->fsdev->path);
    ditem_len = kstrlen(ditem->path);

    path = (char *)kalloc(fsdev_len + ditem_len + 3);
    if (path == NULL)
        return NULL;

    kstrcpy(path, ditem->fsdev->path);
    if (ditem->path[0] != '/')
        path[fsdev_len] = '/';
    kstrcat(path, ditem->path);

    return path;
}

/* 初始化虚拟文件系统的目录项管理模块 */
void ditem_init (void)
{
    int i;

    for(i=0; i<DIR_HASH_BUFF_SIZE; i++)
    {
        list_init(&ditem_hashlist[i]);
    }
}

