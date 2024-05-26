/*
 * 专门用于测试文件系统相关的功能
 */
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "file.h"
#include "list.h"
#include "fcntl.h"
#include "fs.h"

void tc_fsapi (void);
void tc_fsDevice (void);
void tc_fsFile (void);

/* 可以在初始化阶段进行测试的用例 */
void tc_fs_power (void)
{
    // tc_fsapi();
}

/* 只能由进程调用的测试用例 */
void tc_fs_process (void)
{
    // tc_fsDevice();
    tc_fsFile();
}


/************************************************************
****                以下为具体的测试用例
************************************************************/
/* 测试文件系统的设备操作
 * ( 注：该用例需要在进程中调用 )
 */
void tc_fsDevice (void)
{
    int fd;
    char rbuf[32];
    char wbuf[32];

    kmemset(wbuf, 0, 32);
    kstrcpy(wbuf, "\r\n tc_fsDevice !\r\n");

    /* 查看设备打开与读写流程是否正常 */
    fd = vfs_open("console", O_RDWR, S_IRWXU);
    vfs_write (fd, wbuf, kstrlen(wbuf));
    vfs_read(fd, rbuf, 1);
    vfs_close(fd);

    /* 测试进程的标准输入 */
    vfs_read (STD_INPUT, rbuf, 1);

    /* 测试进程的标准输出 */
    vfs_write(STD_OUTPUT, wbuf, kstrlen(wbuf));
}


/* 测试 ramfs 文件系统的函数接口 */
#if 1
static void _tc_ramfsPathParser (char *path)
{
    char parent[256], file[128], *pStr = NULL;

    kprintf ("PATH: %s\r\n", path);

    pStr = path_getfirst(path, file);
    kprintf ("first pStr = %s \r\n", pStr);
    kprintf ("first file = %s\r\n", file);

    path_getlast (path, parent, file);
    kprintf("last path = %s\r\n", parent);
    kprintf("last file = %s\r\n", file);
}
void tc_fsapi (void)
{
 #if 1
    /* 测试 ramfs 中字符串处理的接口 */
    char path[256];

    kstrcpy(path, "/home/HSunix/kernel");
    _tc_ramfsPathParser(path);

    kstrcpy(path, "/usr");
    _tc_ramfsPathParser(path);

    kstrcpy(path, "/");
    _tc_ramfsPathParser(path);
 #elif 0

 #endif
}
#endif

/* 测试文件系统的文件操作 */
void tc_fsFile (void)
{
    int fd;
    int len;
    char rbuf[50];
    char wbuf[50];

    kmemset(rbuf, 0, 50);
    kmemset(wbuf, 0, 50);
    kstrcpy(wbuf, "tc_fsFile runing !");
    len = kstrlen(wbuf);
    if (len <= 0)
        return;

    fd = vfs_open("/text.txt", O_RDWR | O_CREAT, S_IRWXU);
    vfs_write (fd, wbuf, len);
    vfs_read(fd, rbuf, len);
    vfs_close(fd);

    kprintf ("file read: %s\r\n", rbuf);
}


