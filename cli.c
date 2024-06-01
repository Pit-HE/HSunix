
#include "defs.h"
#include "cli.h"


/* 命令行模块信息管理结构体 */
typedef struct cli_manage_info
{
    ringbuf_t   cb;
    int         init;
}cliInfo_t;

/* 记录当前命令行界面的信息 */
cliInfo_t cliState;



/* 初始化整个命令行模块 */
void init_cli (void)
{
    char *buf;

    buf = (char *)kalloc(CLI_CMD_BUFF_SIZE);
    if (buf != NULL)
    {
        cliState.init = 1;
        kRingbuf_init(&cliState.cb, buf, CLI_CMD_BUFF_SIZE);
    }
}

/* 命令行模块的入口函数 */
void cli_main (void)
{
    char ch, *cmd;
    int  ret;

    if (cliState.init == 0)
        return;
    ch = console_rChar();
    if (ch < 0)
        return;

    if ((ch != '\r') && (ch != '\n'))
    {
        kRingbuf_putChar(&cliState.cb, ch);
    }
    else
    {
        kprintf ("\r\n");
        cmd = kalloc(CLI_CMD_BUFF_SIZE);
        kRingbuf_get(&cliState.cb, cmd, CLI_CMD_BUFF_SIZE);

        cli_exec(cmd, &ret);

        kRingbuf_clean(&cliState.cb);
        kprintf("admin:~/ ");
    }
}

