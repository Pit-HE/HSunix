/*
 * 1、向指定文件输入数据
 * 2、打印指定文件中的数据
 */
#include "libc.h"


int main (int argc, char *argv[])
{
    int fd, len, val;
    char tmpbuf[33];

    if ((1 >= argc) || (argc > 3))
    {
        printf ("cat: Incorrect number of entries !\r\n");
        return -1;
    }

    /* 打开指定路径下的文件对象 */
    fd = open(argv[1], O_RDWR, S_IRWXU);
    if (fd < 0)
    {
        printf ("cat: File does not exist !\r\n");
        return -1;
    }

    if (argc == 2)
    {/* 打印文件内容 */
        do
        {
            /* 读取文件对象的信息 */
            len = read(fd, tmpbuf, 32);
            tmpbuf[len] = '\0';

            /* 打印读取到的信息 */
            if (len)
                printf("%s", tmpbuf);
            if (len < 32)
                printf("\r\n");
        }while(len == 32);
    }
    else
    {/* 将数据写入到文件 */
        len = strlen(argv[2]);

        /* 将偏移指针移动到文件末尾 */
        lseek(fd, 0, SEEK_END);

        /* 将数据写入文件 */
        val = write(fd, argv[2], len);
        if (val != len)
            return -1;
    }

    /* 关闭已打开的文件 */
    close(fd);
    return 0;
}
