/*
 * 命令行字符串解析的功能接口
 *
 * 命令的格式，例如：
 *      ls /home -al
 * 该命令为：ls
 * 字符串中携带的参数有两个：/home 与 -al
 */
#include "defs.h"
#include "cli.h"



/* 解析字符串中携带的命令，并返回命令的起始地址
 *
 * 1、将命令与参数之间的空格改为'\0'
 * 
 * 返回值：字符串携带的参数的起始地址
 */
char *cli_parse_cmd (char *str)
{
    char *ptr = str;
    int len, index = 0;

    if ((str == NULL) && (*str != ' '))
        return NULL;
    len = kstrlen(str);

    /* 不允许溢出操作 */
    while(index < len)
    {
        /* 判断是否到命令与参数的分界点 */
        if (ptr[index] == ' ')
        {
            ptr[index] = '\0';
            index += 1;
            return (ptr + index);
        }

        /* 避免命令长度过大 */
        if (index >= CLI_CMD_LEN)
            return NULL;

        index++;
    }

    return NULL;
}

/* 解析字符串中包含的命令行参数 
 *
 * 1、命令的每个参数以空格作为分界
 * 2、只处理传入的命令参数，所以传入的 cmd 不包含命令本身
 * 3、对字符串进行分割，将空格修改为 '\0'，让数组 argv 指向
 *    每一个参数的字符串起始地址
 * 
 * param：包含命令参数的字符串
 * argv：存放命令参数的数组
 * 
 * 返回值：参数的数量
 */
int cli_parse_parameter (char *param, char *argv[CLI_ARG_MAX-1])
{
    int argc = 0, index = 0, str_len = 0;
    char *ptr = param;

    if (argv == NULL)
        return -1;
    if (param == NULL)
        return 0;

    str_len = kstrlen(param);

    while (index < str_len)
    {
        /* 跳过空格 */
        while((*ptr == ' ') && (index < str_len))
        {
            ptr ++;
            index ++;
        }
        /* 是否已经完成所有字符的解析 */
        if (index >= str_len)
            break;

        /* 限制参数数量 */
        if (argc >= CLI_ARG_MAX)
            break;

        if (*ptr == '"')
        {
            /* 处理字符串 */
            ptr++;
            index++;
            argv[argc] = ptr;
            argc += 1;

            /* 查找字符串的结束标志 */
            while ((*ptr != '"') && (index < str_len))
            {
                ptr++;
                index++;
            }
            /* 是否已经完成所有字符的解析 */
            if (index >= str_len)
                break;

            *ptr = '\0';
            ptr++;
            index++;
        }
        else
        {
            /* 处理普通参数 */
            argv[argc] = ptr;
            argc += 1;

            /* 查找参数结束的位置 */
            while ((*ptr != ' ') && (index < str_len))
            {
                ptr ++;
                index ++;
            }
            /* 是否已经完成所有字符的解析 */
            if (index >= str_len)
                break;

            *ptr = '\0';
            ptr++;
            index++;
        }
    }

    return argc;
}

