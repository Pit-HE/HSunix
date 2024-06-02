/*
 * 虚拟文件系统对外的操作接口
 */
#include "defs.h"
#include "file.h"
#include "kerror.h"
#include "fcntl.h"
#include "dirent.h"



/* 初始化虚拟文件系统 */
void init_vfs (void)
{
    /* 初始化实体 ramfs 文件系统 */
    void dfs_ramfs_init (void);
    dfs_ramfs_init();

    /* 初始化虚拟文件系统的目录项模块 */
    init_ditem();

    /* 设置根文件系统 */
    vfs_mount("ramfs", "/", 
        O_RDWR | O_CREAT | O_DIRECTORY, NULL);
}

/* 文件系统对外接口：打开指定路径的文件 */
int vfs_open (char *path, unsigned int flags, unsigned int mode)
{
    int fd;
    struct File *file = NULL;

    fd = fd_alloc();
    if (fd < 0)
    {
        kErr_printf("fail: vfs_open alloc fd !\r\n");
        return -1;
    }

    file = fd_get(fd);
    if (file == NULL)
    {
        kErr_printf("fail: vfs_open get fd !\r\n");
        return -1;
    }

    if (0 > file_open(file, (char *)path, flags, mode))
    {
        kErr_printf("fail: vfs_open open file !\r\n");
        fd_free(fd);
        return -1;
    }

    return fd;
}

/* 文件系统对外接口：关闭已打开的文件 */
int vfs_close (int fd)
{
    int ret;
    struct File *file = NULL;

    file = fd_get(fd);
    if (file < 0)
    {
        kErr_printf("fail: vfs_close get fd !\r\n");
        return -1;
    }

    ret = file_close(file);
    if (0 > ret)
    {
        kErr_printf("fail: vfs_close close file !\r\n");
        return -1;
    }

    fd_free(fd);

    return ret;
}

/* 文件系统对外接口：将数据写入指定文件中 */
int vfs_write (int fd, void *buf, int len)
{
    int ret;
    struct File *file = NULL;

    file = fd_get(fd);
    if (file < 0)
    {
        kErr_printf("fail: vfs_write get fd !\r\n");
        return -1;
    }

    ret = file_write(file, buf, len);
    if (0 > ret)
    {
        kErr_printf("fail: vfs_write write file !\r\n");
        return -1;
    }

    return ret;
}

/* 文件系统对外接口：从指定文件中读取指定长度的数据 */
int vfs_read (int fd, void *buf, int len)
{
    int ret;
    struct File *file = NULL;

    file = fd_get(fd);
    if (file < 0)
    {
        kErr_printf("fail: vfs_read get fd !\r\n");
        return -1;
    }

    ret = file_read(file, buf, len);
    if (ret < 0)
    {
        kErr_printf("fail: vfs_read read file !\r\n");
        return -1;
    }

    return ret;
}

int vfs_unlink (char *path)
{
    int fd, ret = -1;
    struct File *file;

    if (path == NULL)
    {
        kErr_printf("fail: vfs_unlink path !\r\n");
        return -1;
    }

    fd = fd_alloc();
    if (fd < 0)
    {
        kErr_printf("fail: vfs_open alloc fd !\r\n");
        return -1;
    }

    file = fd_get(fd);
    if (file == NULL)
    {
        kErr_printf("fail: vfs_open get fd !\r\n");
        return -1;
    }

    /* 获取要处理的文件对象 */
    if (0 > file_open(file, (char *)path, O_RDWR, S_IRWXU))
    {
        kErr_printf("fail: vfs_open open file !\r\n");
        fd_free(fd);
        return -1;
    }

    /* 按照不同的类型，处理不同的释放处理 */
    switch (file->inode->type)
    {
        case INODE_DEVICE:
            ret = 0;
            dev_put(file->inode->dev);
            break;
        case INODE_FILE:
        case INODE_DIR:
            ret = file_unlink(path);
            break;
        case INODE_PIPO:
            break;
        default: break;
    }

    return ret;
}

/* 初始化进程的文件相关项*/
int vfs_pcbInit (ProcCB *pcb, char *path)
{
    if (pcb == NULL)
        return -1;

    /* 传入的必须是绝对路径 */
    if ((path == NULL) || (*path != '/'))
        return -1;

    /* 创建进程工作路径的字符串 */
    pcb->cwd = (char *)kalloc(kstrlen(path)+1);
    if ( pcb->cwd == NULL)
        return -1;
    kstrcpy(pcb->cwd, path);

    /* 设置进程的根节点 */
    pcb->root = file_alloc();
    if (pcb->root == NULL)
    {
        kfree(pcb->cwd);
        return -1;
    }
    if (-1 == file_open(pcb->root, path, \
            O_DIRECTORY | O_RDWR | O_CREAT, S_IRUSR))
    {
        file_free(pcb->root);
        kfree(pcb->cwd);
        return -1;
    }

    /* 为进程申请默认大小的文件描述符数组空间 */
    if (0 > fdTab_alloc(pcb))
    {
        file_close(pcb->root);
        file_free(pcb->root);
        kfree(pcb->cwd);
        return -1;
    }

    return 0;
}

/* 反初始化进程的文件相关项 */
int vfs_pcbdeinit (ProcCB *pcb)
{
    if (pcb == NULL)
        return -1;

    file_close(pcb->root);

    /* 释放进程占用的内存资源 */
    file_free(pcb->root);
    kfree(pcb->cwd);
    fdTab_free(pcb);

    return 0;
}

/* 对外提供的文件系统挂载接口
 *  ( 传入的必须是绝对路径 )
 */
int vfs_mount(char *fsname, char *path, 
        unsigned int flag, void *data)
{
    int ret;

    if ((fsname == NULL) || (path == NULL))
        return -1;

    /* 挂载第一个实体文件系统 */
    ret = fsdev_mount(fsname, path, flag, data);

    return ret;
}
