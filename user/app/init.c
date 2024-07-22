/* 
 * 内核的第一个进程
 */
#include "libc.h"
#include "shell.h"

int main (int argc, char *argv[])
{
    int pid, sh_pid;
    char  sh_path[32];
    char *sh_argv[1] = {NULL};

    while(1)
    {
        sh_pid = fork();
        if (sh_pid < 0)
        {
            printf ("Fail: shell fork !\r\n");
            exit(1);
        }
        if (sh_pid == 0)
        {
            strcpy(sh_path, "/bin/");
            strcat(sh_path, argv[0]);
            printf ("init: sh_path = %s\r\n", sh_path);

            exec(sh_path, sh_argv);
            printf ("init: exec shell fail !\r\n");
        }

        while(1)
        {
            pid = wait(NULL);
            if (pid == sh_pid)
            {
                /* 重新加载 shell 进程 */
                printf ("shell exit, restart !\r\n");
                break;
            }
            else
            {
                /* 有子进程被释放 */
                printf ("init: free pid = %d child process !\r\n", pid);
            }
        }
    }

    return -1;
}
