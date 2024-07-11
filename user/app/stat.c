/*
 * 显示指定文件对象的信息
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    if ((argc != 2) ||
        (argv[1] == NULL))
        return -1;

    return stat(argv[1]);
}
