
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
    if (0 > file_open(file, path, O_DIRECTORY | O_RDONLY, S_IRUSR))
    {
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

    return 0;
}

/* 读取指定目录内的目录项信息
 * 会单次读取多个目录项，使得下一次 readir
 * 可以直接从 DIR 的 buf 中获取，避免每次
 * 都要操作虚拟文件系统去读写磁盘
 */
struct dirent *readdir(DIR *dir)
{
    int ret;
    struct dirent *dirent = NULL;

    if (dir == NULL)
        return NULL;

    do
    {
        /* DIR 中是否已经有读取到的 dirent */
        if (dir->bufsize)
            dir->index += sizeof(struct dirent);

        /* 是否需要操作虚拟文件系统读取新的 dirent 信息 */
        if ((dir->bufsize == 0) || 
            (dir->index >= dir->bufsize))
        {
            ret = file_getdents(fd_get(dir->fd), 
                          (struct dirent *) dir->buf,
                          sizeof(dir->buf));
            if (ret <= 0)
                return NULL;
            
            dir->bufsize = ret;
            dir->index = 0;
        }
        /* 获取 DIR 中存储的 dirent 对象 */
        dirent = (struct dirent *)&dir->buf[dir->index];

        /* 跳过文件目录默认存在的 '.' 与 '..' */
        if ((kstrcmp(dirent->name, ".") != 0) ||
            (kstrcmp(dirent->name, "..") != 0))
        {
            break;
        }
    }while(dirent);

    return dirent;
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
    
    /* 将偏移值设置为 0 */
    if (file->off > offset)
    {
        file_lseek(file, offset, SEEK_SET);
    }

    /* 一直移动偏移值，直到其与入参相同 */
    while(file->off < offset)
    {
        if (NULL == readdir(dir))
            break;
    }
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
int mkdir (char *path, unsigned int mode)
{
    int ret;
    struct File *file;

    if (path == NULL)
        return -1;

    file = file_alloc();
    if (file == NULL)
        return -1;
    
    /* 创建目录对象 */
    ret = file_open(file, path, O_CREAT|O_DIRECTORY, mode);
    if (ret > 0)
    {
        file_close(file);
    }

    file_free(file);

    return ret;
}

/* 更改当前进程的工作目录 */
int chdir(char *path)
{
    DIR *dir;
    ProcCB *pcb;

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
    closedir(dir);

    /* 设置进程的任务控制块 */
    kDISABLE_INTERRUPT();
    pcb->root = fd_get(dir->fd);
    path_setcwd(path);
    kENABLE_INTERRUPT();

    return 0;
}
