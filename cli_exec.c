/*
 * 处理命令行界面执行指定命令的功能
 */
#include "defs.h"
#include "cli.h"


/* 解析传入的字符串，并执行相应的命令 */
int cli_exec (char *cmd, int *retp)
{
    char *param;
    int   argc;
    char *argv[CLI_ARG_MAX];
    cmd_function func;
    char *func_name;

    if (cmd == NULL)
        return -1;
    func_name = cmd;

    /* 获取命令参数字符串的首地址 */
    param = cli_parse_cmd(cmd);
    if (param == NULL)
        return -1;
    
    /* 解析参数的内容 */
    kmemset(argv, 0, sizeof(argv));
    argc = cli_parse_parameter(param, argv);
    if (argc == 0)
        return -1;
    
    /* 获取该命令所对应的函数 */
    func = cli_cmd_get(func_name);

    /* 执行该命令所对应的函数 */
    *retp = func(argc, argv);
    return 0;
}

