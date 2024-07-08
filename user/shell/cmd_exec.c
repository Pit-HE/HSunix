/*
 * 处理命令行界面执行指定命令的功能
 */
#include "libc.h"
#include "shell.h"


/* 解析传入的字符串，并执行相应的命令 
 * 1、传入的 cmd 会通过插入 '\0' 来分割为多个字符串
 */
int cmd_exec (char *cmd, int *retp)
{
    char *param = NULL;
    int   argc = 0, ret = 0;
    char *argv[CLI_ARG_MAX];
    char  path[32];

    if (cmd == NULL)
        return -1;
    memset(argv, 0, sizeof(argv));

    /* 获取命令参数字符串的首地址 */
    param = cli_parse_cmd(cmd);

    /* 将命令存储在第一位 */
    argv[argc++] = cmd;

    /* 解析参数的内容 */
    argc += cli_parse_parameter(param, &argv[1]);

    strcpy(path, "/bin/");
    strcat(path, argv[0]);

    /* TODO: */
    printf ("exec: %s\r\n", path);
    // exec(path, argv);

    /* 是否需要返回值 */
    if (retp != NULL)
        *retp = ret;

    return 0;
}
