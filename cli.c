/*
 * 命令行交互模块对外的功能接口
 */
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



/* 清空屏幕的内容,将光标移动到左上角 */
void cli_clear (void)
{
    kprintf("\x1b[2J\x1b[H");
}

void cli_logo (void)
{
    cli_clear();
    kprintf("HSunix kernel is booting.\r\n");
    kprintf("\r\n");
    kprintf(" _    _    _____                   _         \r\n");
    kprintf("| |  | |  / ____|                 (_)        \r\n");
    kprintf("| |__| | | (___    _   _   _ __    _  __  __ \r\n");
    kprintf("|  __  |  \\___ \\  | | | | |  _ \\  | | \\ \\/ / \r\n");
    kprintf("| |  | |  ____) | | |_| | | | | | | |  >  <  \r\n");
    kprintf("|_|  |_| |_____/   \\__ _| |_| |_| |_| /_/\\_\\ \r\n");
    kprintf("\r\n\r\n");
    kprintf("\033[1;33m HSunix running in RISC64-V architecture.\033[0m \r\n");
    kprintf("\033[1;33m Usr 'help' to list all command.\033[0m \r\n");
}

/* 命令行模块的入口函数 */
void cli_main (void)
{
    char ch, *cmd;
    int  val, endflag = 0;

    if (cliState.init == 0)
        return;

    while(1)
    {
        /* 每次命令执行完成，重新输出当前命令交互的提示 */
        if (endflag == 0)
        {
            endflag++;
            kprintf("admin:%s$ ", getProcCB()->cwd);
        }

        /* 从控制台获取单个字符 */
        ch = console_rChar();
        if (ch < 0)
            return;

        if ((ch != '\r') && (ch != '\n'))
        {
            if ((ch == 0x7F) || (ch == 0x08))
            {
                /* 处理输入删除和回车的情况 */
                if (0 <= kRingbuf_delChar(&cliState.cb))
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
            cmd = (char *)kalloc(CLI_CMD_BUFF_SIZE);
            kRingbuf_get(&cliState.cb, cmd, CLI_CMD_BUFF_SIZE);

            cli_exec(cmd, &val);

            kRingbuf_clean(&cliState.cb);
            kfree(cmd);
            endflag = 0;
        }
    }
}



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
    cli_logo();
}
