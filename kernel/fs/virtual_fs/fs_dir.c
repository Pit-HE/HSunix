/*
 *  管理并操作文件系统的目录项
 */
#include "defs.h"
#include "file.h"
#include "fcntl.h"
#include "kstring.h"


/* 定义存放目录项的哈希数组 */
#define DIR_HASH_BUFF_SIZE      32
ListEntry_t ditem_hashlist[DIR_HASH_BUFF_SIZE];



/* 计算目录项所对应的哈希值 */
static int ditem_hash(struct FsDevice *fsdev, const char *path)
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
static char *ditem_pathformat(struct FsDevice *fsdev, 
        const char *path)
{
    uint len;

    /* 获取文件系统挂载路径的长度 */
    len = kstrlen(fsdev->path);
    if (len == 0)
        return NULL;

    /* 当前路径是否为文件系统的挂载路径 */
    if (0 == kstrncmp(fsdev->path, path, len))
        path += len;

    return (char *)path;
}

/* 查找指定的目录项是否已经存在 */
static struct DirItem *ditem_find (struct FsDevice *fsdev, 
        const char *path)
{
    int index;
    ListEntry_t *list = NULL;
    struct DirItem *ditem = NULL;
    char *str = NULL, root_path[] = {"/"};

    if ((fsdev == NULL) || (path == NULL))
        return NULL;

    /* 去除字符串中包含的文件系统挂载路径 */
    str = ditem_pathformat(fsdev, path);
    if (*str == '\0')
    {
       /* 处理传入的是文件系统挂载目录的情况
        * ( 挂载路径的目录项名字为 '/' )
        */
       str = root_path;
    }

    /* 获取所寻目录项的哈希值 */
    index = ditem_hash(fsdev, str);

    /* 遍历哈希数组成员的链表，获取指定的目录项 */
    list_for_each(list, &ditem_hashlist[index])
    {
        ditem = list_container_of(list, struct DirItem, list);
        if ((ditem->fsdev == fsdev) &&
            (0 == kstrcmp(ditem->path, str)))
        {
            return ditem;
        }
    }
    return NULL;
}

/* 创建新的目录项，并初始化 */
struct DirItem *ditem_alloc (struct FsDevice *fsdev, 
        const char *path)
{
    struct DirItem *ditem = NULL;

    if ((fsdev == NULL) || (path == NULL))
        return NULL;

    ditem = (struct DirItem *)kalloc(sizeof(struct DirItem));
    if (ditem == NULL)
        return NULL;

    /* 跳过 path 中包含的关于文件系统挂载的路径 */
    path = ditem_pathformat(fsdev, path);
    if (*path == '\0')
    {
        /* 处理传入的是文件系统挂载目录的情况
         * ( 设置文件系统挂载路径目录项的path )
         */
        ditem->path = kstrdup("/");
    }
    else
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

    return ditem;
}

/* 释放已经申请的目录项的内存空间 */
int ditem_free (struct DirItem *dir)
{
    if (dir == NULL)
        return -1;
    if ((dir->magic != DIRITEM_MAGIC) &&
        (dir->ref == 0))
    dir->magic = 0;

    kfree(dir->path);
    kfree(dir);

    return 0;
}

/* 将目录项添加到模块的哈希数组中 (与 ditem_del 成对使用) */
static void ditem_add (struct DirItem *ditem)
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

/* 将目录项从哈希数组链表中移除 (与 ditem_add 成对使用) */
static void ditem_del (struct DirItem *ditem)
{
    if ((ditem == NULL) ||
        (ditem->magic != DIRITEM_MAGIC))
        return;

    kDISABLE_INTERRUPT();
    list_del_init(&ditem->list);
    ditem->state = DITEM_ALLOC;
    kENABLE_INTERRUPT();
}

/* 创建新的目录项 (传入的必须是绝对路径) */
struct DirItem *ditem_create (struct FsDevice *fsdev, 
        const char *path)
{
    struct DirItem *ditem = NULL;

    if ((fsdev == NULL) || (path == NULL))
        return NULL;

    /* 创建新的目录项 */
    ditem = ditem_alloc(fsdev, path);
    if (ditem == NULL)
        return NULL;

    ditem->ref += 1;

    ditem_add(ditem);

    return ditem;
}

/* 删除已创建的目录项 (与 ditem_create 成对使用) */
int ditem_destroy (struct DirItem *ditem)
{
    if ((ditem == NULL) || (ditem->magic != DIRITEM_MAGIC))
        return -1;
    if (ditem->ref > 1)
    {
        ditem->ref -= 1;
        return -1;
    }
    ditem->ref = 0;

    /* 从链表移除 */
    ditem_del(ditem);

    /* 释放目录项占用的资源 */
    ditem_free(ditem);

    return 0;
}

/* 获取已存在的目录项 (传入的必须是绝对路径) */
struct DirItem *ditem_get (struct FsDevice *fsdev, 
        const char *path)
{
    char *ap_path = NULL;
    struct Inode *inode = NULL;
    struct DirItem *ditem = NULL;

    if ((fsdev == NULL) || (path == NULL))
        return NULL;

    /* 确认目录项是否已经存在 */
    ditem = ditem_find(fsdev, path);
    if (ditem == NULL)
    {
        /* 申请新的 inode 节点 */
        inode = inode_getfs(fsdev, 0, 0);
        if (inode == NULL)
            return NULL;

        ap_path = ditem_pathformat(fsdev, path);

        /* 获取与该目录项对应的实体文件系统成员 */
        if (0 > fsdev->fs->fsops->lookup(fsdev, inode, ap_path))
        {
            inode_put(inode);
            return NULL;
        }

        /* 创建新的目录项记录已存在的文件成员 */
        ditem = ditem_create(fsdev, ap_path);
        if (ditem == NULL)
        {
            inode_put(inode);
            return NULL;
        }
        ditem->inode = inode;
    }
    ditem->ref += 1;

    return ditem;
}

/*      释放已经获取的目录项
 * (目录项不会删除，需要继续保留在内存中)
 */
void ditem_put (struct DirItem *ditem)
{
    if (ditem == NULL)
        return;
    if ((ditem->magic != DIRITEM_MAGIC) ||
        (ditem->ref == 0))
        return;

    ditem->ref -= 1;
    ditem->state = DITEM_CLOSE;
}

/* 获取目录项的绝对路径 */
char *ditem_path (struct DirItem *ditem)
{
    char *path = NULL;
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
void init_ditem (void)
{
    int i;

    for(i=0; i<DIR_HASH_BUFF_SIZE; i++)
    {
        list_init(&ditem_hashlist[i]);
    }
}

