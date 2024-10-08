
#include "defs.h"
#include "file.h"
#include "fs.h"
#include "fcntl.h"
#include "dirent.h"




/* 仅打开指定路径的目录对象 */
DIR *opendir(char *path)
{
    DIR *dir = NULL;

    if (path == NULL)
        return NULL;

    /* 申请目录对象的内存空间 */
    dir = (DIR *)kalloc(sizeof(DIR));
    if (dir == NULL)
        return NULL;   

    dir->fd = vfs_open(path, O_DIRECTORY | O_RDWR, S_IRWXU);
    if (dir->fd < 0)
        return NULL;

    return dir;
}

/* 关闭已经打开的目录对象 */
int closedir(DIR *dir)
{
    if (dir == NULL)
        return -1;
    
    vfs_close(dir->fd);
    kfree(dir);

    return 0;
}

/* 读取目录对象内的当前目录项信息 */
struct dirent *readdir(DIR *dir)
{
    if (dir == NULL)
        return NULL;

    if(0 >= vfs_getdirent(dir->fd,
        dir->buf, sizeof(struct dirent)))
        return NULL;

    return (struct dirent *)dir->buf;
}

/* 设置当前目录对象的读写位置 */
void seekdir(DIR *dir, long offset)
{
    if ((dir == NULL) || (dir->fd <= STD_ERROR))
        return;

    vfs_lseek(dir->fd, offset, SEEK_SET);
}

/* 在指定路径创建目录对象 */
int mkdir (char *path, uint mode)
{
    int fd;

    if (path == NULL)
        return -1;

    fd = vfs_open(path, O_CREAT | O_DIRECTORY | O_RDWR, mode);
    if (fd >= 0)
        vfs_close(fd);

    return fd;
}

int mkfile (char *path, uint flag, uint mode)
{
    int fd;

    if (path == NULL)
        return -1;
    
    fd = vfs_open(path, flag, mode);
    if (fd < 0)
        return -1;

    vfs_close(fd);

    return 0;
}

/* 更改当前进程的工作目录 */
int chdir(char *path)
{
    int fd;

    if (path == NULL)
        return -1;

    /* 确认该目录项存在 */
    fd = vfs_open(path, O_RDONLY | O_DIRECTORY, S_IRWXU);
    if (fd < 0)
        return -1;
    vfs_close(fd);

    return path_setcwd(path);
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

    return vfs_unlink(path);
}
