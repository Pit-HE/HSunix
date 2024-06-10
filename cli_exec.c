/*
 * 处理命令行界面执行指定命令的功能
 */
#include "defs.h"
#include "cli.h"


/* 解析传入的字符串，并执行相应的命令 
 * 1、传入的 cmd 会通过插入 '\0' 来分割为多个字符串
 */
int cli_exec (char *cmd, int *retp)
{
    char *param = NULL;
    int   argc = 0, ret = 0;
    char *argv[CLI_ARG_MAX];
    cmd_function func = NULL;

    if (cmd == NULL)
        return -1;
    kmemset(argv, 0, sizeof(argv));

    /* 获取命令参数字符串的首地址 */
    param = cli_parse_cmd(cmd);

    /* 将命令存储在第一位 */
    argc += 1;
    argv[0] = cmd;

    /* 解析参数的内容 */
    argc += cli_parse_parameter(param, &argv[1]);

    /* 获取该命令所对应的函数 */
    func = cli_cmd_get(cmd);
    if (func == NULL)
    {
        kErrPrintf("%s:command not found\r\n", cmd);
        kErrPrintf("   Enter 'help' to view the command list\r\n");
        return -1;
    }

    /* 执行该命令所对应的函数 */
    ret = func(argc, argv);
    if (ret < 0)
        kErrPrintf ("Command execution failure\r\n");

    /* 是否需要返回值 */
    if (retp != NULL)
        *retp = ret;

    return 0;
}
