/*
 * 处理命令行界面执行指定命令的功能
 */
#include "libc.h"
#include "shell.h"


/* 解析传入的字符串，并执行相应的命令 
 * 1、传入的 cmd 会通过插入 '\0' 来分割为多个字符串
 */
int cmd_exec (char *cmd)
{
    int argc = 0;//, pid;
    char *param = NULL;
    char *argv[CLI_ARG_MAX];
    char  path[32];

    if (cmd == NULL)
        return -1;
    memset(argv, 0, sizeof(argv));

    /* 解析要处理的命令字符串 */
    param = cli_parse_cmd(cmd);
    argc += cli_parse_parameter(param, argv);

    /* 获取 bin 文件夹中对应的命令 */
    strcpy(path, "/bin/");
    strcat(path, cmd);

    // pid = fork();
    // if (pid < 0)
    // {
    //     printf ("fail: shell fork !\r\n");
    //     exit(1);
    // }
    // if (pid == 0)
    // {
        exec (path, argv);
    //     printf ("fail: shell exec !\r\n");
    //     exit(1);
    // }

    return 0;
}
