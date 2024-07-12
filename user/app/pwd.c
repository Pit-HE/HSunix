/*
 * 显示当前进程的工作路径
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    int ret;
    char buf[64];

    ret = getcwd(buf, 64);
    if (ret >= 0)
        printf ("%s\r\n", buf);

    return ret;
}
