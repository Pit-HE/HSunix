
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
            example: mkdir /home/book"
    },
    {/* cd */
        "cd",
        "Modify the process work path\r\n    \
            example: cd /home | cd /home/.. | cd /"
    },
    {/* ls */
        "ls",
        "List all the objects under the folder\r\n    \
            example: ls | ls /bin"
    },
    {/* mkfile */
        "mkfile",
        "Create the specified file\r\n    \
            example: mkfile b.txt | mkfile /home/b.txt"
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
            example: cat /home/a.a  (Display the file content) \r\n    \
                     cat /home/a.a abc123  (Write the ('abc123')data to the file)"
    },
    {/* rm */
        "rm",
        "Deletes the specified file or directory\r\n    \
            example: rm /home/a.a"
    },
    {/* rmdir */
        "rmdir",
        "Deletes the specified directory\r\n    \
            example: rmdir /home/tmp1"
    },
    {/* fsync */
        "fsync",
        "Refresh the cache to disk for the specified file\r\n    \
            example: fsync /home/a.a"
    },
    {/* fstatfs */
        "fstatfs",
        "Obtain the file system information of the file\r\n    \
            example: fstatfs /home/a.a"
    },
    {/* stat */
        "stat",
        "Displays the information of the specified file\r\n    \
            example: stat /home/a.a"
    },
    {/* rename */
        "rename",
        "Modify the name of the existing file\r\n    \
            example: rename /home/a.a /home/b.b"
    },
    {/* mount */
        "mount",
        "Hang the message system to the specified path\r\n    \
            example: mount tmpfs /tmp"
    },
    {/* unmount */
        "unmount",
        "Uninstall the file system of the specified path\r\n    \
            example: unmount /tmp"
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
