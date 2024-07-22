/*
 * 挂载新的文件系统到指定路径
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    int ret;

    if (argc != 3)
    {
        printf ("mount: Incorrect number of entries !\r\n");
        return -1;
    }
    if ((argv[1] == NULL) || (argv[2] == NULL))
    {
        printf ("mount: Input in an illegal file name\r\n");
        return -1;
    }

    ret = mount(argv[1], argv[2]);
    return ret;
}
