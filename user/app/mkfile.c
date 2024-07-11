/*
 * 创建文件
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    int fd;

    if ((argc != 2) ||
        (argv[1] == NULL))
        return -1;
    
    fd = open(argv[1], O_CREAT | O_RDWR, S_IRWXU);
    if (fd > 0)
        close(fd);

    return fd;
}

