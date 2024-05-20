/*
 * 对文件的路径字符串进行解析的模块
 */
#include "defs.h"
#include "file.h"
#include "fcntl.h"

/* 记录操作系统上电默认使用的文件系统 */
struct FileSystem *default_fs = NULL;


/* 获取指定文件所在的父 inode */
// static struct Inode *path_parent (char *path)
// {
//     return 0;
// }

/* 获取指定文件所对应的 inode，
 * 若路径所对应实体文件文件的 inode 成员不存在则创建
 */
struct Inode *path_parser (char *path)
{
    ProcCB *pcb;
    struct Inode *inode;

    if (path == NULL)
        return NULL;
    pcb = getProcCB();

    // 是否为绝对路径
    if (*path == '/')
        inode = default_fs->data;
    else
        inode = pcb->cwd;


    return inode;
}


/* 挂载系统默认使用的文件系统 */
int path_init (void)
{
    if (-1 == fsdev_mount("ramfs", "rootfs", O_RDWR, NULL))
        return -1;

    default_fs = fsdev_get("rootfs");
    if (default_fs == NULL)
        return -1;

    return 0;
}

