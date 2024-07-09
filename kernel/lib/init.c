/*
 * init 进程相关的源码
 */
#include "defs.h"
#include "fcntl.h"


void init_main (void)
{
    // struct ProcCB  *tempPCB = NULL;

    kENABLE_INTERRUPT();

    /* 设置根文件系统 */
    vfs_mount("diskfs", "/", O_RDWR | O_CREAT | O_DIRECTORY, NULL);

    /* 挂载管理设备的文件系统 */
    vfs_mount("devfs", "/dev", O_RDWR | O_CREAT | O_DIRECTORY, NULL);
    mkfile("/dev/a.a", O_CREAT|O_RDWR, S_IRWXU);

    /* 挂载用于测试的内存文件系统 */
    vfs_mount("ramfs", "/home", O_RDWR | O_CREAT | O_DIRECTORY, NULL);
    mkfile("/home/a.a", O_CREAT|O_RDWR, S_IRWXU);

    // tempPCB = create_kthread("user", user_main);
    // vfs_pcbInit(tempPCB, "/");
    // proc_wakeup(tempPCB);

    while (1)
    {
        do_wait(NULL);
    }
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

void test_main (void)
{
    kENABLE_INTERRUPT();

    while(1)
    {
        cli_main();
    }
}

void user_main (void)
{
    char *argv[8] = {0};
    char *param0 = "Hello World";
    char *param1 = "user_main";

    kENABLE_INTERRUPT();
    argv[0] = param0;
    argv[1] = param1;
    argv[2] = NULL;

    /* 设置进入用户空间后要执行的函数 */
    // do_exec(NULL, "/bin/init", argv);
    do_exec(NULL, "/bin/test", argv);

    getProcCB()->trapFrame->a0 = 2;
    trap_userret();
}
