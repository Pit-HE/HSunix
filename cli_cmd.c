/*
 * 记录当前命令行模块所有可执行的命令
 */
#include "defs.h"
#include "cli.h"



int test (int argc, char *argv[])
{
    int i;

    kprintf("\r\n");
    for (i=0; i<argc; i++)
    {
        kprintf(argv[i]);
        kprintf(" ");
    }
    kprintf("\r\n");

    return 0;
}










struct cli_cmd func_list[] = 
{
    {
        &test,
        "test",
        "test function"
    }
};
#define CMD_LIST_LEN sizeof(func_list)/sizeof(func_list[0])

cmd_function cli_cmd_get (char *name)
{
    struct cli_cmd *cmdlist = func_list;
    cmd_function func = NULL;
    int i;

    for (i=0; i<CMD_LIST_LEN; i++)
    {
        if ( 0 == kstrcmp(cmdlist->name, func_list[i].name))
        {
            func = cmdlist->func;
            break;
        }
    }

    return func;
}

