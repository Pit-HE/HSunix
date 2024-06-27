/*
 * init 进程相关的源码
 */
#include "defs.h"
#include "fcntl.h"


void init_main (void)
{
    /* 设置根文件系统 */
    vfs_mount("diskfs", "/", O_RDWR | O_CREAT | O_DIRECTORY, NULL);

    /* 挂载管理设备的文件系统 */
    vfs_mount("devfs", "/dev", O_RDWR | O_CREAT | O_DIRECTORY, NULL);
    mkfile("/dev/a.a", O_CREAT|O_RDWR, S_IRWXU);

    /* 挂载用于测试的内存文件系统 */
    vfs_mount("ramfs", "/home", O_RDWR | O_CREAT | O_DIRECTORY, NULL);
    mkfile("/home/a.a", O_CREAT|O_RDWR, S_IRWXU);


    while (1)
    {
        cli_main();
    }
}


void idle_main (void)
{
    while(1)
    {
        do_scheduler();
        do_defuncter();
    }
}


void user_main (void)
{
    char *argv[8] = {0};
    char buf[] = "Hello World";
    struct ProcCB *pcb = getProcCB();

    argv[0] = buf;
    argv[1] = NULL;

    /* 设置进入用户空间后要执行的函数 */
    do_exec(pcb, "/bin/test", argv);

    trap_userret();
}
