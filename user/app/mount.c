/*
 * 挂载新的文件系统到指定路径
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    int ret;

    if ((argc != 2) || 
        (argv[0] == NULL) || (argv[1] == NULL))
        return -1;

    ret = mount(argv[0], argv[1]);
    return ret;
}
