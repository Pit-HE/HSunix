/* 
 * 内核的第一个进程
 */
#include "libc.h"
#include "shell.h"

int main (int argc, char *argv[])
{
    printf("Init function is running !\r\n");

    init_shell();

    while(1)
    {
        shell_main();
    }
    return 0;
}
