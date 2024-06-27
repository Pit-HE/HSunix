/*
 * 记录当前命令行模块所有可执行的命令
 */
#include "defs.h"
#include "cli.h"
#include "fs.h"
#include "dirent.h"
#include "fcntl.h"
#include "proc.h"



/* 创建文件夹 */ 
int cmd_mkdir (int argc, char *argv[])
{
    if ((argc != 2) || (argv[1] == NULL))
        return -1;

    return mkdir(argv[1], S_IRWXU);
}

/* 修改进程的工作路径 */
int cmd_cd (int argc, char *argv[])
{
    if ((argc != 2) || (argv[1] == NULL))
        return -1;

    return k_chdir(argv[1]);
}

/* 列举当前目录下的所有对象 */
int cmd_ls (int argc, char *argv[])
{
    DIR *dir = NULL;
    uint count = 0;
    struct dirent *dirent = NULL;
    struct ProcCB *pcb = getProcCB();

    if (argc > 2)
        return -1;

    switch (argc)
    {
        case 1: /* 列举当前目录 */
            dir = opendir(pcb->cwd);
            break;
        case 2: /* 列举指定路径 */
            if (argv[1] == NULL)
                return -1;
            dir = opendir(argv[1]);
            break;
        default:/* 暂不认可其他参数 */
            break;
    }

    /* 将目录项偏移指针移动到开头 */
    seekdir(dir, 0);
    do
    {
        dirent = readdir(dir);
        if (dirent != NULL)
        {
            /* 显示名字 */
            kprintf ("%-15s",dirent->name);
            count += 1;
            if (count >= 5)
            {
                count = 0;
                kprintf ("\r\n");
            }
        }
    }while(dirent != NULL);
    closedir(dir);

    if (count != 0)
        kprintf("\r\n");
    return 0;
}

/* 创建文件 */
int cmd_mkfile (int argc, char *argv[])
{
    if ((argc != 2) || (argv[1] == NULL))
        return -1;

    return mkfile(argv[1], O_CREAT|O_RDWR, S_IRWXU);
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
    if ((argc != 2) || (argv[1] == NULL))
        return -1;

    kprintf("%s\r\n", argv[1]);
    return 0;
}

/* 向指定文件输入数据 */
int cmd_cat (int argc, char *argv[])
{
    int fd, len, val;
    char tmpbuf[33];

    if ((1 >= argc) || (argc > 3))
        return -1;

    /* 打开指定路径下的文件对象 */
    fd = vfs_open(argv[1], O_RDWR, S_IRWXU);
    if (fd < 0)
        return -1;

    if (argc == 2)
    {/* 打印文件内容 */
        do
        {
            /* 读取文件对象的信息 */
            len = vfs_read(fd, tmpbuf, 32);
            tmpbuf[len] = '\0';

            /* 打印读取到的信息 */
            if (len)
                kprintf("%s", tmpbuf);
            if (len < 32)
                kprintf("\r\n");
        }while(len == 32);
    }
    else
    {/* 将数据写入到文件 */
        len = kstrlen(argv[2]);

        /* 将偏移指针移动到文件末尾 */
        vfs_lseek(fd, 0, SEEK_END);

        /* 将数据写入文件 */
        val = vfs_write(fd, argv[2], len);
        if (val != len)
            return -1;
    }

    /* 关闭已打开的文件 */
    vfs_close(fd);
    return 0;
}

/* 删除指定的文件或目录 */
int cmd_rm (int argc, char *argv[])
{
    int fd;

    if ((argc != 2) || (argv[1] == NULL))
        return -1;

    /* 确认要删除的文件存在 */
    fd = vfs_open(argv[1], O_RDONLY, S_IRWXU);
    if (fd < 0)
        return -1;
    vfs_close(fd);
    
    return vfs_unlink(argv[1]);
}

/* 删除指定的目录 */
int cmd_rmdir (int argc, char *argv[])
{
    if ((argc != 2) || (argv[1] == NULL))
        return -1;
    
    return k_rmdir(argv[1]);
}

/* 同步文件系统内的缓存到磁盘 */
int cmd_fsync (int argc, char *argv[])
{
    int fd, ret;

    if ((argc != 2) || (argv[1] == NULL))
        return -1;
    
    fd = vfs_open(argv[1], O_RDWR, S_IRWXU);
    if (fd < 0)
        return -1;

    ret = vfs_fsync(fd);
    vfs_close(fd);
    
    return ret;
}

/* 获取指定文件路径对象所属文件系统的信息 */
int cmd_fstatfs (int argc, char *argv[])
{
    int fd, ret = -1;
    struct statfs fsbuf;

    if ((argc != 2) || (argv[1] == NULL))
        return -1;
    
    fd = vfs_open(argv[1], O_RDWR, S_IRWXU);
    if (fd < 0)
        return -1;

    /* 获取当前文件所属文件系统的信息 */
    ret = vfs_fstatfs(fd, &fsbuf);
    vfs_close(fd);

    kprintf("    fs name:  %s\r\n", fsbuf.name);
    kprintf("    fs total: %d\r\n", fsbuf.f_total);
    kprintf("    fs bsize: %d\r\n", fsbuf.f_bsize);
    kprintf("    fs block: %d\r\n", fsbuf.f_block);
    kprintf("    fs bfree: %d\r\n", fsbuf.f_bfree);

    return ret;
}

/* 显示指定文件的信息 */
int cmd_stat (int argc, char *argv[])
{
    int fd, ret = -1;
    struct stat buf;

    if ((argc != 2) || (argv[1] == NULL))
        return -1;
    
    fd = vfs_open(argv[1], O_RDONLY, S_IRWXU);
    if (fd < 0)
        return -1;

    ret = vfs_stat(fd, &buf);
    vfs_close(fd);

    kprintf("    name: %s\r\n", buf.name);
    kprintf("    size: %d\r\n", buf.size);
    if (buf.type == VFS_DIR)
        kprintf("    type: dir\r\n");
    else
        kprintf("    type: file\r\n");
    kprintf("    fsname: %s\r\n", buf.fsname);

    return ret;
}

/* 修改指定文件的名字 */
int cmd_rename (int argc, char *argv[])
{
    if (argc != 3)
        return - 1;
    if ((argv[1] == NULL) || (argv[2] == NULL))
        return -1;

    /* 调用接口直接修改文件对象的名字 */
    return vfs_rename(argv[1], argv[2]);
}

/* 挂载新的文件系统 */
int cmd_mount (int argc, char *argv[])
{
    int ret;

    if ((argc != 3) || 
        (argv[1] == NULL) || (argv[2] == NULL))
        return -1;

    ret = vfs_mount(argv[1], argv[2], 
        O_RDWR | O_CREAT | O_DIRECTORY, NULL);
    return ret;
}

/* 卸载已经挂载的文件系统 */
int cmd_unmount (int argc, char *argv[])
{
    if ((argc != 2) || (argv[1] == NULL))
        return -1;

    return vfs_unmount(argv[1]);
}

/* 清除命令行交互的屏幕 */
int cmd_clear(int argc, char *argv[]) 
{
    if (argc != 1)
        return -1;

    cli_clear();
    return 0;
}

int cmd_exec (int argc, char *argv[])
{
    if (argc < 2)
        return -1;

    // do_exec(NULL, "/fs/cat", argv);
    proc_wakeup(do_kthread("user", user_main));
    return 0;
}

/***********************************************************
 *      记录所有交互命令的数组，以及解析该数组的命令行接口
***********************************************************/
int cmd_help (int argc, char *argv[]);

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
        "To create a new document clip\r\n    \
            example: mkdir sys | mkdir /usr/sys"
    },
    {/* cd */
        &cmd_cd,
        "cd",
        "Modify the process work path\r\n    \
            example: cd / | cd /home | cd /home/.."
    },
    {/* ls */
        &cmd_ls,
        "ls",
        "List all the objects under the folder\r\n    \
            example: ls | ls /home | ls /home/book/.."
    },
    {/* mkfile */
        &cmd_mkfile,
        "mkfile",
        "Create the specified file\r\n    \
            example: mkfile aa.a | mkfile /usr/aa.a"
    },
    {/* pwd */
        &cmd_pwd,
        "pwd",
        "Show current process work path\r\n    \
            example: pwd"
    },
    {/* echo */
        &cmd_echo,
        "echo",
        "Displays the specified string content\r\n    \
            example: echo abc123"
    },
    {/* cat */
        &cmd_cat,
        "cat",
        "Display the file content or write the data to the file\r\n    \
            example: cat a.a  (Display the file content) \r\n    \
                     cat a.a abc123  (Write the data to the file)"
    },
    {/* rm */
        &cmd_rm,
        "rm",
        "Deletes the specified file or directory\r\n    \
            example: rm a.a | rm /bin/a.a"
    },
    {/* k_rmdir */
        &cmd_rmdir,
        "k_rmdir",
        "Deletes the specified directory\r\n    \
            example: k_rmdir usr | k_rmdir /usr/tmp1"
    },
    {/* fsync */
        &cmd_fsync,
        "fsync",
        "Refresh the cache to disk for the specified file\r\n    \
            example: fsync a.a | fsync /bin/a.a"
    },
    {/* fstatfs */
        &cmd_fstatfs,
        "fstatfs",
        "Obtain the file system information of the file\r\n    \
            example: fstatfs a.a | fstatfs /bin/a.a"
    },
    {/* stat */
        &cmd_stat,
        "stat",
        "Displays the information of the specified file\r\n    \
            example: stat a.a | stat /bin/a.a"
    },
    {/* rename */
        &cmd_rename,
        "rename",
        "Modify the name of the existing file\r\n    \
            example: rename a.a b.b"
    },
    {/* mount */
        &cmd_mount,
        "mount",
        "Hang the message system to the specified path\r\n    \
            example: mount tmpfs /fs | mount diskfs /fs"
    },
    {/* unmount */
        &cmd_unmount,
        "unmount",
        "Uninstall the file system of the specified path\r\n    \
            example: unmount /fs"
    },
    {/* clear */
        &cmd_clear,
        "clear",
        "Clear screen and move cursor to top-left corner\r\n"
    },
    {/* cmd_exe */
        &cmd_exec,
        "exec",
        "sdafsddfs"
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
    int i, name_len, cmd_len;
    cmd_function func = NULL;

    if (name == NULL)
        return NULL;
    name_len = kstrlen(name);

    /* 遍历命令列表 */
    for (i=0; i<CMD_LIST_LEN; i++)
    {
        cmd_len = kstrlen(func_list[i].name);

        /* 确认当前函数是否为所寻对象 */
        if (0 != kstrncmp(func_list[i].name, name, cmd_len))
            continue;

        /* 确认命令名长度是否一致 */
        if (cmd_len != name_len)
            continue;

        /* 获取指定的命令 */
        func = func_list[i].func;
        break;
    }

    return func;
}


