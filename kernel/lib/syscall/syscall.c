/*
 * 主要提供处理系统调用接口相关的基本功能
 */
#include "syspriv.h"
#include "syscall.h"
#include "defs.h"
#include "proc.h"


/* 获取指定的系统调用的入参 */
static inline uint64 arg_raw (int num)
{
    struct ProcCB *pcb = getProcCB();
    switch (num)
    {
        case 0:  return pcb->trapFrame->a0;
        case 1:  return pcb->trapFrame->a1;
        case 2:  return pcb->trapFrame->a2;
        case 3:  return pcb->trapFrame->a3;
        case 4:  return pcb->trapFrame->a4;
        case 5:  return pcb->trapFrame->a5;
        case 6:  return pcb->trapFrame->a6;
        default: return 0;
    }
}

/* 从指定的入参中读取 int 类型的数据 */
void arg_int (int num, int *data)
{
    *data = (int)arg_raw(num);
}

/* 从指定的入参中读取地址类型的数据 */
void arg_addr (int num, uint64 *addr)
{
    *addr = arg_raw(num);
}

/* 从指定的入参中读取字符串类型的数据 */
int arg_str (int num, char *buf, int len)
{
    struct ProcCB *pcb = getProcCB();
    return (uvm_copyin(pcb->pgtab, buf, arg_raw(num), len));
}


/* 系统调用的总列表 */
static uint64 (*_syscall[])(void) =
{
    [SYS_exit   ]   sys_exit,
    [SYS_fork   ]   sys_fork,
    [SYS_wait   ]   sys_wait,
    [SYS_exec   ]   sys_exec,
    [SYS_yield  ]   sys_yield,
    [SYS_kill   ]   sys_kill,
    [SYS_getpid ]   sys_getpid,
    [SYS_putc   ]   sys_putc,
    [SYS_getc   ]   sys_getc,
    [SYS_gettime]   sys_gettime,
    [SYS_sleep  ]   sys_sleep,
    [SYS_suspend]   sys_suspend,
    [SYS_resume ]   sys_resume,
    [SYS_open   ]   sys_open,
    [SYS_close  ]   sys_close,
    [SYS_read   ]   sys_read,
    [SYS_write  ]   sys_write,
    [SYS_lseek  ]   sys_lseek,
    [SYS_stat   ]   sys_stat,
    [SYS_fsync  ]   sys_fsync,
    [SYS_dup    ]   sys_dup,
    [SYS_getdirent] sys_getdirent,
    [SYS_unlink ]   sys_unlink,
    [SYS_chdir  ]   sys_chdir,
    [SYS_fstatfs]   sys_fstatfs,
    [SYS_mount  ]   sys_mount,
    [SYS_umount ]   sys_umount,
    [SYS_getcwd ]   sys_getcwd,
    [SYS_rename ]   sys_rename,
};
#define SYSCALL_NUM (sizeof(_syscall)/sizeof(_syscall[0]))

/* 系统调用的总入口，复制调用具体的接口函数 */
void do_syscall (void)
{
    struct ProcCB *pcb = getProcCB();
    uint code = pcb->trapFrame->a7;

    if ((code < SYSCALL_NUM) && (_syscall[code] != NULL))
    {
        pcb->trapFrame->a0 = _syscall[code]();
    }
    else
    {
        kprintf ("This system call is invalid !\r\n");
        pcb->trapFrame->a0 = -1;
    }
}
