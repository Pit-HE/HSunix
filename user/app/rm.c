/*
 * 删除指定的文件或目录
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    int fd;

    if (argv[0] == NULL)
        return -1;

    /* 确认要删除的文件存在 */
    fd = open(argv[0], O_RDONLY, S_IRWXU);
    if (fd < 0)
        return -1;
    close(fd);
    
    return unlink(argv[0]);
}
