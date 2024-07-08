/*
 * 内核 exec 函数的封装
 */
#include "clib.h"

int main (int argc, char *argv[])
{
    printf("Exec function is running !\r\n");

    exec("/bin/test", NULL);
    while(1)
    {
        sleep(1000);
    }
    return 0;
}

