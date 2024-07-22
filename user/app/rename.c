/*
 * 修改指定文件的名字
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    if (argc != 3)
    {
        printf ("rename: Incorrect number of entries !\r\n");
        return - 1;
    }
    if ((argv[1] == NULL) || (argv[2] == NULL))
    {
        printf ("rename: Input in an illegal file name\r\n");
        return -1;
    }

    /* 调用接口直接修改文件对象的名字 */
    return rename(argv[1], argv[2]);
}
