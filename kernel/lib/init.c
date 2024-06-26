/*
 * init 进程相关的源码
 */
#include "defs.h"
#include "fcntl.h"


/* 在文件系统中创建默认的文件树结构 */
void default_fsdir (void)
{
    mkdir("/fs", S_IRWXU);
    
    mkdir("/usr", S_IRWXU);
    mkdir("/usr/tmp1", S_IRWXU);
    mkdir("/usr/tmp2", S_IRWXU);

    mkdir("/bin", S_IRWXU);
    mkdir("/bin/bin", S_IRWXU);
    mkdir("/bin/bin/stop", S_IRWXU);
    mkfile("/bin/a.a", O_CREAT|O_RDWR, S_IRWXU);
    mkfile("/bin/b.b", O_CREAT|O_RDWR, S_IRWXU);

    mkdir("/home", S_IRWXU);
    mkfile("/home/abc.o", O_CREAT|O_RDWR, S_IRWXU);
    mkfile("/home/123.o", O_CREAT|O_RDWR, S_IRWXU);
    mkdir("/home/study", S_IRWXU);
    mkdir("/home/study/tmp1", S_IRWXU);
    mkdir("/home/study/tmp2", S_IRWXU);
    mkdir("/home/book", S_IRWXU);
    mkdir("/home/book/usr", S_IRWXU);
    mkdir("/home/book/sys", S_IRWXU);
    mkfile("/home/book/a.a", O_CREAT|O_RDWR, S_IRWXU);
    mkfile("/home/book/b.b", O_CREAT|O_RDWR, S_IRWXU);

    mkfile("/a.a", O_CREAT|O_RDWR, S_IRWXU);
    mkfile("/b.b", O_CREAT|O_RDWR, S_IRWXU);
}


void init_main (void)
{
    while (1)
    {
        cli_main();
    }
}

void idle_main (void)
{
    default_fsdir();

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

    /* 设置进入用户空间后要执行的函数 */
    // void user_first (void);
    // pcb->trapFrame->epc = (uint64)user_first;
    do_exec(pcb, "/fs/test", argv);

    trap_userret();
}
