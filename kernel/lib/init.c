/*
 * init 进程相关的源码
 */
#include "defs.h"
#include "fcntl.h"


void init_main (void)
{
    /* 告诉 init 进程要加载的 shell 程序 */
    char *argv[2] = {"sh", NULL};
    kENABLE_INTERRUPT();

    /* 设置根文件系统 */
    vfs_mount("diskfs", "/", O_RDWR | O_CREAT | O_DIRECTORY, NULL);

    /* 挂载管理设备的文件系统 */
    vfs_mount("devfs", "/dev", O_RDWR | O_CREAT | O_DIRECTORY, NULL);
    mkfile("/dev/a.a", O_CREAT|O_RDWR, S_IRWXU);

    /* 挂载用于测试的内存文件系统 */
    vfs_mount("ramfs", "/home", O_RDWR | O_CREAT | O_DIRECTORY, NULL);
    mkfile("/home/a.a", O_CREAT|O_RDWR, S_IRWXU);

    do_exec("/bin/init", argv);
    trap_userret();
}


void idle_main (void)
{
    k_enable_all_interrupt();

    while(1)
    {
        do_scheduler();
        do_defuncter();
    }
}
