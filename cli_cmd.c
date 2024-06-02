/*
 * 记录当前命令行模块所有可执行的命令
 */
#include "defs.h"
#include "cli.h"
#include "fs.h"
#include "dirent.h"
#include "fcntl.h"

int cmd_help (int argc, char *argv[]);



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
    struct dirent *dirent = NULL;
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
        dirent = readdir(dir);
        if (dirent != NULL)
        {
            /* 显示类型 */
            if (dirent->type == INODE_DIR)
                kprintf ("d--   ");
            else if (dirent->type == INODE_FILE)
                kprintf ("f--   ");

            /* 显示大小 */
            if (dirent->datasize < 10)
                kprintf (" %d    ", dirent->datasize);
            else if (dirent->datasize < 100)
                kprintf (" %d   ", dirent->datasize);
            else if (dirent->datasize < 1000)
                kprintf (" %d  ", dirent->datasize);

            /* 显示名字 */
            kprintf ("%s\r\n",dirent->name);
        }
    }while(dirent != NULL);
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

/* 输出文件内的数据 */
int cmd_echo (int argc, char *argv[])
{
    int fd;
    int rlen;
    char tmpbuf[33];

    if (argc != 2)
        return -1;
    
    fd = vfs_open(argv[1], O_RDWR, S_IRWXU);
    if (fd < 0)
        return -1;

    do
    {
        kmemset(tmpbuf, 0, 32);
        rlen = vfs_read(fd, tmpbuf, 32);

        if (rlen)
            kprintf("%s", tmpbuf);
        if (rlen < 32)
            kprintf("\r\n");
    }while(rlen == 32);
    vfs_close(fd);

    return 0;
}

/* 向指定文件输入数据 */
int cmd_cat (int argc, char *argv[])
{
    int fd, wlen, val;

    if (argc != 3)
        return -1;
    
    fd = vfs_open(argv[1], O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0)
        return -1;

    wlen = kstrlen(argv[2]);
    val = vfs_write(fd, argv[2], wlen);
    if (val != wlen)
        return -1;

    vfs_close(fd);

    return 0;
}

/* 删除指定的文件或目录 */
int cmd_rm (int argc, char *argv[])
{
    if (argc != 2)
        return -1;

    return vfs_unlink(argv[1]);
}

/* 删除指定的目录 */
int cmd_rmdir (int argc, char *argv[])
{
    if (argc != 2)
        return -1;
    
    return rmdir(argv[1]);
}

/***********************************************************
 * 
***********************************************************/
struct cli_cmd func_list[] = 
{
    {/* help */
        &cmd_help,
        "help",
        "list all command"
    },
    {/* mkdir */
        &cmd_mkdir,
        "mkdir",
        "You can use it to create folders"
    },
    {/* cd */
        &cmd_cd,
        "cd",
        "Modify the process work path"
    },
    {/* ls */
        &cmd_ls,
        "ls",
        "List all the objects under the folder"
    },
    {/* mkfile */
        &cmd_mkfile,
        "mkfile",
        "Create the specified file"
    },
    {/* pwd */
        &cmd_pwd,
        "pwd",
        "Show current process work path"
    },
    {/* echo */
        &cmd_echo,
        "echo",
        "Data in the output file"
    },
    {/* cat */
        &cmd_cat,
        "cat",
        "Writes data to the specified file"
    },
    {/* rm */
        &cmd_rm,
        "rm",
        "Deletes the specified file or directory"
    },
    {/* rmdir */
        &cmd_rmdir,
        "rmdir",
        "Deletes the specified directory"
    },
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
    int i, name_len, cmd_len;
    cmd_function func = NULL;

    if (name == NULL)
        return NULL;
    name_len = kstrlen(name);

    /* 遍历命令列表 */
    for (i=0; i<CMD_LIST_LEN; i++)
    {
        cmd_len = kstrlen(func_list[i].name);

        /*  */
        if (0 != kstrncmp(func_list[i].name, name, cmd_len))
            continue;

        /*  */
        if (cmd_len != name_len)
            continue;

        /* 获取指定的命令 */
        func = func_list[i].func;
        break;
    }

    return func;
}


