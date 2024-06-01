
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
    int  ret, endflag = 0;

    if (cliState.init == 0)
        return;

    while(1)
    {
        if (endflag == 0)
        {
            endflag++;
            kprintf("admin:~/ ");
        }

        ch = console_rChar();
        if (ch < 0)
            return;

        if ((ch != '\r') && (ch != '\n'))
        {
            /* 处理输入删除和回车的情况 */
            if ((ch == 0x7F) || (ch == 0x08))
            {
                if (0 != kRingbuf_delChar(&cliState.cb))
                    kprintf("\b \b");
            }
            else
            {
                /* 正常存储输入的字符 */
                kRingbuf_putChar(&cliState.cb, ch);
            }
        }
        else
        {
            /* 输入了回车键，开始执行命令行内容 */
            kprintf ("\r\n");
            cmd = kalloc(CLI_CMD_BUFF_SIZE);
            kRingbuf_get(&cliState.cb, cmd, CLI_CMD_BUFF_SIZE);

            cli_exec(cmd, &ret);

            kRingbuf_clean(&cliState.cb);
            endflag = 0;
        }
    }
}

