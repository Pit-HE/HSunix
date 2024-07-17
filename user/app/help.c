
#include "libc.h"


struct help_info
{
    char name[CLI_CMD_LEN];
    char *info;
};

struct help_info help_list[] = 
{
    {/* help */
        "help",
        "list all command"
    },
    {/* mkdir */
        "mkdir",
        "To create a new document clip\r\n    \
            example: mkdir sys | mkdir /usr/sys"
    },
    {/* cd */
        "cd",
        "Modify the process work path\r\n    \
            example: cd / | cd /home | cd /home/.."
    },
    {/* ls */
        "ls",
        "List all the objects under the folder\r\n    \
            example: ls | ls /home | ls /home/book/.."
    },
    {/* mkfile */
        "mkfile",
        "Create the specified file\r\n    \
            example: mkfile aa.a | mkfile /usr/aa.a"
    },
    {/* pwd */
        "pwd",
        "Show current process work path\r\n    \
            example: pwd"
    },
    {/* echo */
        "echo",
        "Displays the specified string content\r\n    \
            example: echo abc123"
    },
    {/* cat */
        "cat",
        "Display the file content or write the data to the file\r\n    \
            example: cat a.a  (Display the file content) \r\n    \
                     cat a.a abc123  (Write the data to the file)"
    },
    {/* rm */
        "rm",
        "Deletes the specified file or directory\r\n    \
            example: rm a.a | rm /bin/a.a"
    },
    {/* rmdir */
        "rmdir",
        "Deletes the specified directory\r\n    \
            example: rmdir usr | rmdir /usr/tmp1"
    },
    {/* fsync */
        "fsync",
        "Refresh the cache to disk for the specified file\r\n    \
            example: fsync a.a | fsync /bin/a.a"
    },
    {/* fstatfs */
        "fstatfs",
        "Obtain the file system information of the file\r\n    \
            example: fstatfs a.a | fstatfs /bin/a.a"
    },
    {/* stat */
        "stat",
        "Displays the information of the specified file\r\n    \
            example: stat a.a | stat /bin/a.a"
    },
    {/* rename */
        "rename",
        "Modify the name of the existing file\r\n    \
            example: rename a.a b.b"
    },
    {/* mount */
        "mount",
        "Hang the message system to the specified path\r\n    \
            example: mount tmpfs /fs | mount diskfs /fs"
    },
    {/* unmount */
        "unmount",
        "Uninstall the file system of the specified path\r\n    \
            example: unmount /fs"
    },
    {/* clear */
        "clear",
        "Clear screen and move cursor to top-left corner\r\n"
    },
};
#define CMD_LIST_LEN sizeof(help_list)/sizeof(help_list[0])


/* 打印列表内所有可用的命令 */
int main (int argc, char *argv[])
{
    int i;

    /* 罗列所有的命令信息 */
    for (i=0; i<CMD_LIST_LEN; i++)
    {
        printf("  %s:\r\n",help_list[i].name);
        printf("            ");
        printf("%s\r\n", help_list[i].info);
    }
    return 0;
}
