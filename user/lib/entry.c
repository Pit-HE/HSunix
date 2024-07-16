
#include "libc.h"

extern void init_pthread (void);
extern int main (int argc, char *argv[]);


void _main (int argc, char *argv[])
{
    int ret;

    init_memory();
    init_pthread();

/** 以上代码适用于所有进程运行前的初始化 **/
    ret = main (argc, argv);
    exit(ret);
}
