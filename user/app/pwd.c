/*
 * 显示当前进程的工作路径
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    int ret;
    char buf[32];

    ret = getcwd(buf, 32);
    if (ret >= 0)
        printf ("%s\r\n", buf);

    return ret;
}
