/*
 * 列举当前目录下的所有文件对象
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    DIR *dir = NULL;
    uint count = 0;
    struct dirent *dirent = NULL;

    if (argc > 3)
        return -1;

    switch (argc)
    {
        case 1: /* 列举当前目录 */
            dir = opendir(".");
            break;
        case 2: /* 列举指定路径 */
            if (argv[1] == NULL)
            {
                printf("ls: fail, Null parameter !\r\n");
                return -1;
            }
            dir = opendir(argv[1]);
            break;
        default:/* 暂不认可其他参数 */
            break;
    }

    /* 将目录项偏移指针移动到开头 */
    seekdir(dir, 0);
    do
    {
        dirent = readdir(dir);
        if (dirent != NULL)
        {
            /* 显示名字 */
            printf ("%-15s",dirent->name);
            count += 1;
            if (count >= 5)
            {
                count = 0;
                printf ("\r\n");
            }
        }
    }while(dirent != NULL);
    closedir(dir);

    if (count != 0)
        printf("\r\n");
    return 0;
}