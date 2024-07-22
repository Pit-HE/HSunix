/*
 * 处理命令行界面执行指定命令的功能
 */
#include "libc.h"
#include "shell.h"

/* 对要执行的命令进行分类 
 *
 * 返回值：
 *  1 表示该命令在 shell 进程中执行
 *  0 表示该命令需要创建新进程来执行
 */
int cmd_classify (char *cmd, int argc, char *argv[])
{
    int ret = 0;

    if (strncmp(cmd, "cd", 2) == 0)
    {
        chdir(argv[1]);
        ret = 1;
    }
    else if (strncmp(cmd, "clear", 5) == 0)
    {
        printf("\x1b[2J\x1b[H");
        ret = 1;
    }
    else if (strncmp(cmd, "stat", 4) == 0)
    {
        stat(argv[1]);
        ret = 1;
    }
    return ret;
}

/* 解析传入的字符串，并执行相应的命令 
 * 1、传入的 cmd 会通过插入 '\0' 来分割为多个字符串
 */
int cmd_exec (char *cmd)
{
    int fd;
    int argc, pid;
    char *param = NULL;
    char *argv[CLI_ARG_MAX];
    char  path[32];

    if (cmd == NULL)
        return -1;
    memset(argv, 0, sizeof(argv));

    /* 解析要处理的命令字符串 */
    param = cli_parse_cmd(cmd);
    argc    = 1;
    argv[0] = cmd;
    argc += cli_parse_parameter(param, &argv[1]);

    /* 将命令划分为 shell 处理与创建新进程处理两种 */
    if (0 != cmd_classify(cmd, argc, argv))
        return 0;

    /* 获取 bin 文件夹中对应的命令 */
    strcpy(path, "/bin/");
    strcat(path, cmd);

    /* 确认文件是否存在 */
    fd = open(path, O_RDONLY, S_IRWXU);
    if (fd < 0)
    {
        printf("%s: command not found\r\n", cmd);
        printf("   Enter 'help' to view the command list\r\n");
        return -1;
    }
    close(fd);

    pid = fork();
    if (pid < 0)
    {
        printf ("fail: shell fork !\r\n");
        exit(1);
    }
    if (pid == 0)
    {
        exec (path, argv);
        printf ("fail: shell exec !\r\n");
        exit(1);
    }
    else
    {
        wait(NULL);
    }

    return 0;
}
