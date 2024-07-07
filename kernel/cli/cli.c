/*
 * 命令行交互模块对外的功能接口
 */
#include "defs.h"
#include "cli.h"


/* 命令行模块信息管理结构体 */
struct cli_manage_info
{
    bool        init;
    bool        finish;
    char        chr;
    char        *cmd;
    int         cmd_ret;
    int         buf_index;
    char        buf[CLI_CMD_BUFF_SIZE];
};

/* 记录当前命令行界面的信息 */
struct cli_manage_info shell;



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
    kprintf("\033[1;33m HSunix running in riscv64 architecture.\033[0m \r\n");
    kprintf("\033[1;33m Usr 'help' to list all command.\033[0m \r\n");
}

/* 处理输入的是方向键的情况 */
int cli_directionkey (const char ch)
{
    int ret = 0;
    static uint8 state = 0;

    if (ch == 0x1b)
    {
        ret = 1;
        state = 1;
    }
    else if (state == 1)
    {
        if (ch != 0x5b)
            state = 0;
        else
        {
            ret = 1;
            state = 2;
        }
    }
    else if (state == 2)
    {
        state = 0;
        ret = ch;

        switch (ch)
        {
            case 0x41: /* 上 */
                break;
            case 0x42: /* 下 */
                break;
            case 0x43: /* 右 */
                break;
            case 0x44: /* 左 */
                break;
        }
    }
    return ret;
}

/* 命令行模块的入口函数 */
void cli_main (void)
{
    /* 确认命令行模块已经初始化 */
    while(shell.init == FALSE)
        do_suspend(&shell.init);

    while(1)
    {
        /* 每次命令执行完成，重新输出当前命令交互的提示 */
        if (shell.finish == TRUE)
        {
            shell.finish = FALSE;
            kprintf("\033[1;32madmin\033[0m:\033[1;34m%s\033[0m$ ", getProcCB()->cwd);
        }

        /* 从控制台获取单个字符 */
        shell.chr = console_rChar();
        switch (shell.chr)
        {
            case '\r':
            case '\n':
                /* 输入了回车键，开始执行命令行内容 */
                kprintf ("\r\n");
                shell.buf[shell.buf_index] = '\0';

                /* 处理仅输入回车键的情况 */
                if (shell.buf[0] != '\0')
                {
                    /* 去除字符串前的无效字符 */
                    shell.cmd = shell.buf;
                    while(*shell.cmd == ' ')
                        shell.cmd++;

                    /* 执行命令的处理 */
                    cli_exec(shell.cmd, &shell.cmd_ret);
                }
                shell.finish = TRUE;
                shell.buf_index = 0;
                shell.buf[0] = '\0';
                break;
            case 0x00:
            case 0xFF:
                break;
            case 0x7F:
            case 0x08:
                /* 处理输入删除和回车的情况 */
                if (shell.buf_index > 0)
                {
                    shell.buf_index -= 1;
                    kprintf("\b \b");
                }
                break;
            case '\t':
                /* 处理输入 tab 键的情况 */
                break;
            default:
                /* 处理方向键 */
                if (cli_directionkey(shell.chr))
                    break;

                /* 正常存储输入的字符 */
                shell.buf[shell.buf_index++] = shell.chr;
                /* 同时在界面上显示输入的字符 */
                console_wChar(NULL, shell.chr);
                break;
        }
    }
}



/* 初始化整个命令行模块 */
void init_cli (void)
{
    kmemset(&shell, 0, sizeof(shell));

    shell.init = TRUE;
    shell.finish = TRUE;

    cli_logo();
    do_resume(&shell.init);
}
