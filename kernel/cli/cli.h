/*
 * 命令行交互界面模块共用的头文件
 */
#ifndef __CLI_H__
#define __CLI_H__


/* 当前命令行模块最大支持传入多少个命令参数 */
#define CLI_ARG_MAX     10

/* 命令所占用的最大字符串长度 */
#define CLI_CMD_LEN     10

/* 命令行界面，每次缓存的字符串总长度 */
#define CLI_CMD_BUFF_SIZE   128


typedef int (*cmd_function)(int argc, char *argv[]); 

/* 用于创建命令行模块可以识别的命令对象 */
struct cli_cmd
{
    cmd_function func;
    char name[CLI_CMD_LEN];
    char *info;
};





char *cli_parse_cmd (char *str);
int   cli_parse_parameter (char *param, char *argv[CLI_ARG_MAX-1]);
cmd_function cli_cmd_get (char *name);

void cli_clear (void);
int  cli_exec  (char *cmd, int *retp);
void cli_main  (void);
void init_cli  (void);

#endif
