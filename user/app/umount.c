/*
 * 卸载已经挂载的文件系统
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    if (argv[0] == NULL)
        return -1;

    return umount(argv[0]);
}
