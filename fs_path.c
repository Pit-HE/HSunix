/*
 * 对文件的路径字符串进行解析的模块
 */
#include "defs.h"
#include "file.h"
#include "fcntl.h"

/* 记录操作系统上电默认使用的文件系统 */
struct FsDevice *default_fs = NULL;


/* 当文件路径没有对应的 inode 时则创建新的节点
 * ( 未处理文件路径中间缺少多个中间文件夹的情况 )
 */
int path_createinode (struct DirItem *ditem, char *path)
{
    char parent_path[256];
    struct FsDevice *fsdev = ditem->fsdev;

    /* 创建文件路径所对应的文件节点 */
    if (fsdev->fs->fsops->create == NULL)
        return -1;
    if (fsdev->fs->fsops->lookup == NULL)
        return -1;

    /* 确认该文件路径中的父节点是否存在 */
    fstr_getlast(path, parent_path, NULL);
    if (-1 == fsdev->fs->fsops->lookup(fsdev, ditem->inode, parent_path))
        return -1;

    /* 父节点存在，则表明该文件可以创建 */
    fsdev->fs->fsops->create(fsdev, ditem->inode, path);

    return 0;
}


/* 获取文件路径所对应的 inode */
struct Inode *path_parser (char *path,
    unsigned int flags, enum InodeType type)
{
    struct FileSystem *fs;
    ProcCB *pcb = getProcCB();
    struct Inode *inode = NULL;
    char *abs_path; // 绝对路径

    if ((path == NULL) || (pcb == NULL))
        return NULL;

    if (*path == '/')
    {
        abs_path = path;
        fs = default_fs->fs;
    }
    else
    {
        abs_path = (char *)kalloc(kstrlen(pcb->pwd) + kstrlen(path) + 1);
        if (abs_path == NULL)
            return NULL;
        kstrcpy(abs_path, pcb->pwd);
        kstrcat(abs_path, path);

        fs = pcb->root->inode->fs;
    }
    if ((fs == NULL) || (abs_path == NULL))
        return NULL;

    return inode;
}

/* 挂载系统默认使用的文件系统 */
int path_init (void)
{
    if (-1 == fsdev_mount("ramfs", "/", O_RDWR, NULL))
        return -1;

    default_fs = fsdev_get("/");
    if (default_fs == NULL)
        return -1;

    return 0;
}

