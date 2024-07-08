/* 
 * 内核的第一个进程
 */
#include "clib.h"


int main (int argc, char *argv[])
{
    printf("Init function is running !\r\n");

    while(1)
    {
        sleep(1000);
    }
    return 0;
}
