/*
 * 删除指定的目录
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    DIR *dir = NULL;

    if (argv[0] == NULL)
        return -1;

    /* 确认该目录项存在 */
    dir = opendir(argv[0]);
    if (dir == NULL)
        return -1;
    closedir(dir);

    return unlink(argv[0]);
}
