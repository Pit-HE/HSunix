/*
 * 记录当前命令行模块所有可执行的命令
 */
#include "defs.h"
#include "cli.h"
#include "fs.h"
#include "dirent.h"
#include "fcntl.h"

int cmd_help (int argc, char *argv[]);


/* 测试命令行交互的参数传递功能 */
int cmd_test (int argc, char *argv[])
{
    int i;

    if (argc > 1)
        return 0;

    /* 打印传入的所有参数 */
    for (i=1; i<argc; i++)
    {
        kprintf(argv[i]);
        kprintf(" ");
    }
    kprintf("\r\n");

    return 0;
}

/* 创建文件夹 */ 
int cmd_mkdir (int argc, char *argv[])
{
    if (argc != 2)
        return -1;

    return mkdir(argv[1], S_IRWXU);
}

/* 修改进程的工作路径 */
int cmd_cd (int argc, char *argv[])
{
    if (argc != 2)
        return -1;

    return chdir(argv[1]);
}

/* 列举当前目录下的所有对象 */
int cmd_ls (int argc, char *argv[])
{
    DIR *dir = NULL;
    struct dirent *dp = NULL;
    ProcCB *pcb = getProcCB();

    if (argc > 2)
        return -1;

    switch (argc)
    {
        case 1: /* 列举当前目录 */
            dir = opendir(pcb->cwd);
            break;
        case 2: /* 列举指定路径 */
            dir = opendir(argv[1]);
            break;
        default:/* 暂不认可的参数 */
            break;
    }

    seekdir(dir, 0);
    do
    {
        dp = readdir(dir);
        if (dp != NULL)
            kprintf ("%s   ",dp->name);
    }while(dp != NULL);
    kprintf("\r\n");
    closedir(dir);

    return 0;
}

/* 创建文件 */
int cmd_mkfile (int argc, char *argv[])
{
    int fd;

    if (argc != 2)
        return -1;
    
    fd = vfs_open(argv[1], O_CREAT|O_RDWR, S_IRWXU);
    if (fd < 0)
        return -1;

    vfs_close(fd);

    return 0;
}

/* 显示当前进程的工作路径 */
int cmd_pwd (int argc, char *argv[])
{
    if (argc != 1)
        return -1;
    
    kprintf("%s\r\n", getProcCB()->cwd);
    return 0;
}


/***********************************************************
 * 
***********************************************************/
struct cli_cmd func_list[] = 
{
    /* help */
    {
        &cmd_help,
        "help",
        "list all command"
    },
    /* test */
    {
        &cmd_test,
        "test",
        "test Test the command line parameter"
    },
    /* mkdir */
    {
        &cmd_mkdir,
        "mkdir",
        "You can use it to create folders"
    },
    /* cd */
    {
        &cmd_cd,
        "cd",
        "Modify the process work path"
    },
    /* ls */
    {
        &cmd_ls,
        "ls",
        "List all the objects under the folder"
    },
    /* mkfile */
    {
        &cmd_mkfile,
        "mkfile",
        "Create the specified file"
    },
    /* pwd */
    {
        &cmd_pwd,
        "pwd",
        "Show current process work path"
    }
};
#define CMD_LIST_LEN sizeof(func_list)/sizeof(func_list[0])

/* 打印列表内所有可用的命令 */
int cmd_help (int argc, char *argv[])
{
    int i;

    /* 罗列所有的命令信息 */
    for (i=0; i<CMD_LIST_LEN; i++)
    {
        kprintf("  %s:\r\n",func_list[i].name);
        kprintf("            ");
        kprintf("%s\r\n", func_list[i].info);
    }
    return 0;
}
/* 从命令列表中查找指定的命令 */
cmd_function cli_cmd_get (char *name)
{
    cmd_function func = NULL;
    int i;

    if (name == NULL)
        return NULL;

    /* 遍历命令列表 */
    for (i=0; i<CMD_LIST_LEN; i++)
    {
        if (0 == kstrncmp(func_list[i].name, name, 
            kstrlen(func_list[i].name)))
        {
            /* 获取指定的命令 */
            func = func_list[i].func;
            break;
        }
    }

    return func;
}


