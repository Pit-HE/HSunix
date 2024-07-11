/*
 * 获取指定文件对象所属文件系统的信息
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    int fd, ret = -1;
    struct statfs fsbuf;

    if ((argc != 2) || (argv[1] == NULL))
        return -1;
    
    fd = open(argv[1], O_RDWR, S_IRWXU);
    if (fd < 0)
        return -1;

    /* 获取当前文件所属文件系统的信息 */
    ret = fstatfs(fd, &fsbuf);
    close(fd);

    printf("    fs name:  %s\r\n", fsbuf.name);
    printf("    fs total: %d\r\n", fsbuf.f_total);
    printf("    fs bsize: %d\r\n", fsbuf.f_bsize);
    printf("    fs block: %d\r\n", fsbuf.f_block);
    printf("    fs bfree: %d\r\n", fsbuf.f_bfree);

    return ret;
}
