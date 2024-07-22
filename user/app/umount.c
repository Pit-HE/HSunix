/*
 * 卸载已经挂载的文件系统
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    if (argv[1] == NULL)
    {
        printf ("umount: Incorrect number of entries !\r\n");
        return -1;
    }

    return umount(argv[1]);
}
