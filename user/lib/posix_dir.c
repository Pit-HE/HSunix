
#include "libc.h"
#include "fcntl.h"
#include "syscall.h"
#include "dirent.h"


/* 仅打开指定路径的目录对象 */
DIR *opendir(char *path)
{
    DIR *dir = NULL;

    if (path == NULL)
        return NULL;

    /* 申请目录对象的内存空间 */
    dir = (DIR *)malloc(sizeof(DIR));
    if (dir == NULL)
        return NULL;   

    dir->fd = open(path, O_DIRECTORY | O_RDWR, S_IRWXU);
    if (dir->fd < 0)
        return NULL;

    return dir;
}

/* 关闭已经打开的目录对象 */
int closedir(DIR *dir)
{
    if (dir == NULL)
        return -1;
    
    close(dir->fd);
    free(dir);

    return 0;
}

/* 读取目录对象内的当前目录项信息 */
struct dirent *readdir(DIR *dir)
{
    if (dir == NULL)
        return NULL;

    dir->buf[0] = 0x5A;
    dir->buf[1] = 0x66;
    dir->buf[2] = 0xA5;

    if(0 >= getdirent(dir->fd,
        dir->buf, sizeof(struct dirent)))
        return NULL;

    return (struct dirent *)dir->buf;
}

/* 设置当前目录对象的读写位置 */
void seekdir(DIR *dir, long offset)
{
    if ((dir == NULL) || (dir->fd <= STD_ERROR))
        return;

    lseek(dir->fd, offset, SEEK_SET);
}

