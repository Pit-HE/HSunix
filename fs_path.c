/*
 * 对文件的路径字符串进行解析的模块
 */
#include "defs.h"
#include "file.h"
#include "fcntl.h"

/* 记录操作系统上电默认使用的文件系统 */
struct FileSystem *default_fs = NULL;


/* 解析路径中第一个节点的名字, 并返回剩下的路径内容
 *
 * path: 要解析的路径
 * name：存放第一个节点字符的缓冲区
 */
char *path_getfirst (char *path, char *name)
{
    char *ptr = path;

    if ((path == NULL) || (name == NULL))
        return NULL;

    /* 跳过文件路径上的斜杠 */
    while(*ptr == '/')
        ptr += 1;

    /* 获取第一个节点的字符串 */
    while(*ptr != '/' && *ptr)
        *name++ = *ptr++;

    /* 标记字符串的结束 */
    *name = '\0';

    return ptr;
}

/* 解析路径中最后一个节点的名字，并返回该节点前的父节点路径
 *
 * path: 要解析的路径
 * parentPath: 存放父节点路径的缓冲区
 * name：最后一个节点的名字
 *
 * 返回值：-1为失败
 */
int path_getlast (char *path, char *parentPath, char *name)
{
 #if 0
    char *p_path, *q_path;

    if ((path == NULL) || (parentPath == NULL) || (name == NULL))
        return -1;
    p_path = q_path = path;

    // /* 跳过根目录的斜杠，以及处理只传入根目录的情况 */
    // while(*p_path == '/')
    //     p_path++;
    // if (*p_path == '\0')
    // {
    //     parentPath[0] = '/';
    //     parentPath[1] = '\0';
    //     return 0;
    // }

    // while(1)
    // {
    //     while(*p_path != '/' && *p_path)
    //         p_path++;

    //     if (*p_path != '\0')
    //     {
    //         p_path += 1;    /* 跳过斜杠 */
    //         q_path = p_path;
    //     }
    //     else
    //     {
    //         /* q_path 已停留在子文件名的开头
    //          * p_path 已停留在字符串的结尾处
    //          */
    //         if (q_path == path)
    //         {
    //             parentPath[0] = '/';
    //             parentPath[1] = '\0';
    //             q_path += 1;
    //         }
    //         else
    //         {
    //             kmemcpy(parentPath, path, q_path - path - 1);
    //             parentPath[q_path - path - 1] = '\0';
    //         }
    //         kmemcpy(name, q_path, p_path - q_path);
    //         name[p_path - q_path] = '\0';
    //         break;
    //     }
    // }
 #else
    char *p_path;

    if ((path == NULL) || (parentPath == NULL) || (name == NULL))
        return -1;

    p_path = kstrrchr(path, '/');

    if (p_path == path)
    {
        parentPath[0] = '/';
        parentPath[1] = '\0';
    }
    else
    {
        kmemcpy(parentPath, path, p_path - path - 1);
        parentPath[p_path - path - 1] = '\0';
    }
    kstrcpy(name, p_path + 1);
 #endif

    return 0;
}


/* 当文件路径没有对应的 inode 时则创建新的节点
 * ( 要处理传入的文件路径中间缺少了很多个文件夹的情况 )
 */
int path_createinode (struct Inode *inode,
        struct Inode *boot_inode, char *path)
{
    struct FileSystem *fs = inode->fs;

    /* 创建文件路径所对应的文件节点 */
    if (fs->fsops->create == NULL)
        return -1;

    /* TODO：遍历文件路径，确保路径上的每个文件夹都存在，若不存在则创建它 */

    return 0;
}


/* 获取文件路径所对应的 inode */
struct Inode *path_parser (char *path,
    unsigned int flags, enum InodeType type)
{
    struct FileSystem *fs;
    ProcCB *pcb = getProcCB();
    struct Inode *inode, *boot_inode;

    if ((path == NULL) || (pcb == NULL))
        return NULL;

    inode = inode_alloc();
    if (inode == NULL)
        return NULL;

    if (*path == '/')
    {
        /* 绝对路径 */
        boot_inode = default_fs->root;
        if (path[1] == '\0')
            return boot_inode;
    }
    else
    {
        /* 工作路径 */
        boot_inode = pcb->pwd->inode;
    }
    fs = boot_inode->fs;
    if (fs == NULL)
        return NULL;

    /* 确认文件路径对应的 inode 是否存在 */
    if (fs->fsops->lookup != NULL)
    {
        if (-1 == fs->fsops->lookup(fs, inode, path))
        {
            /* 创建路径对应的文件节点，
               以及其路径上的所有文件夹 */
            if (flags & O_CREAT)
            {
                inode_init(inode, flags, fs->fops, type);
                inode->fs = fs;

                if (-1 == path_createinode (inode, boot_inode,path))
                    inode_free(inode);
            }
            return NULL;
        }
    }

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

