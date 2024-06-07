
#include "defs.h"
#include "file.h"
#include "fs.h"
#include "fcntl.h"
#include "dirent.h"




/* 仅打开指定路径的目录对象 */
DIR *opendir(char *path)
{
    DIR *dir = NULL;
    struct File *file = NULL;

    if (path == NULL)
        return NULL;

    /* 申请目录对象的内存空间 */
    dir = (DIR *)kalloc(sizeof(DIR));
    if (dir == NULL)
        return NULL;   

    /* 获取文件描述符 */
    dir->fd = fd_alloc();
    if (dir->fd < 0)
        return NULL;
    
    /* 获取文件描述符所对应的文件对象 */
    file = fd_get(dir->fd);
    if (file == NULL)
        return NULL;

    /* 打开该文件对象 */
    if (0 > file_open(file, path, O_DIRECTORY | O_RDWR, S_IRWXU))
    {
        fd_free(dir->fd);
        return NULL;
    }

    /* 确认文件对象的类型是否正确 */
    if (file->inode->type != INODE_DIR)
    {
        file_close(file);
        fd_free(dir->fd);
        return NULL;
    }

    return dir;
}

/* 关闭已经打开的目录对象 */
int closedir(DIR *dir)
{
    struct File *file = NULL;

    if (dir == NULL)
        return -1;
    
    /* 获取目录对象所对应的文件对象 */
    file = fd_get(dir->fd);
    if (file == NULL)
        return -1;

    if (0 > file_close(file))
        return -1;

    /* 释放申请的文件描述符 */
    fd_free(dir->fd);
    kfree(dir);

    return 0;
}

/* 读取目录对象内的当前目录项信息 */
struct dirent *readdir(DIR *dir)
{
    if (dir == NULL)
        return NULL;

    if (0 >= file_getdents(fd_get(dir->fd), 
            (struct dirent *) dir->buf, 
            sizeof(struct dirent)))
        return NULL;

    return (struct dirent *)dir->buf;
}

/* 设置当前目录对象的读写位置 */
void seekdir(DIR *dir, long offset)
{
    struct File *file = NULL;

    if ((dir == NULL) || (dir->fd <= STD_ERROR))
        return;
    
    file = fd_get(dir->fd);
    if (file == NULL)
        return;

    file_lseek(file, offset, SEEK_SET);
}

/* 获取当前目录对象的读写位置 */
long telldir(DIR *dir)
{
    struct File *file = NULL;

    if ((dir == NULL) || (dir->fd <= STD_ERROR))
        return - 1;
    
    /* 获取文件描述符 */
    file = fd_get(dir->fd);
    if (file == NULL)
        return -1;

    return file->off;
}

/* 在指定路径创建目录对象 */
int mkdir (char *path, uint mode)
{
    int ret;
    struct File *file;

    if (path == NULL)
        return -1;

    file = file_alloc();
    if (file == NULL)
        return -1;
    
    /* 创建目录对象 */
    ret = file_open(file, path, 
            O_CREAT | O_DIRECTORY | O_RDWR, mode);
    if (ret >= 0)
    {
        file_close(file);
    }

    file_free(file);

    return ret;
}

int mkfile (char *path, uint flag, uint mode)
{
    int fd;

    if (path == NULL)
        return -1;
    
    fd = vfs_open(path, O_CREAT|O_RDWR, S_IRWXU);
    if (fd < 0)
        return -1;

    vfs_close(fd);

    return 0;
}

/* 更改当前进程的工作目录 */
int chdir(char *path)
{
    DIR *dir = NULL;
    ProcCB *pcb = NULL;
    struct File *file = NULL;

    if (path == NULL)
        return -1;

    /* 获取当前进程的控制块 */
    pcb = getProcCB();
    if (pcb == NULL)
        return -1;

    /* 确认该目录项存在 */
    dir = opendir(path);
    if (dir == NULL)
        return -1;

    if (0 <= path_setcwd(path))
    {
        file = fd_get(dir->fd);

        /* 设置进程的任务控制块 */
        kDISABLE_INTERRUPT();
        pcb->root->off   = file->off;
        pcb->root->fops  = file->fops;
        pcb->root->inode = file->inode;
        pcb->root->ditem = file->ditem;
        kENABLE_INTERRUPT();
    }
    closedir(dir);

    return 0;
}

/* 删除指定的文件目录 */
int rmdir (char *path)
{
    DIR *dir = NULL;

    if (path == NULL)
        return -1;

    /* 确认该目录项存在 */
    dir = opendir(path);
    if (dir == NULL)
        return -1;
    closedir(dir);

    return file_unlink(path);
}
