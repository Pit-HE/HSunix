/*
 * 修改当前进程的工作路径
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    return chdir(argv[1]);
}

