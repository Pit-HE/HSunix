/*
 * 在指定路径创建目录对象
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    int fd;

    if ((argc != 2) || (argv[1] == NULL))
    {
        printf ("mkdir: Error parameter !\r\n");
        return -1;
    }

    fd = open(argv[1], O_CREAT | O_DIRECTORY | O_RDWR, S_IRWXU);
    if (fd > 0)
        close(fd);

    return fd;
}
