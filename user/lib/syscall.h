/*
 * 记录所有的系统调用编号
 */
#ifndef __SYSTEM_CALL_H__
#define __SYSTEM_CALL_H__


#include "types.h"


enum system_call_code
{
    SYS_exit = 0,
    SYS_fork,
    SYS_wait,
    SYS_exec,
    SYS_yield,
    SYS_kill = 5,
    SYS_getpid,
    SYS_putc,
    SYS_getc,
    SYS_gettime,  
    SYS_sleep = 10,
    SYS_suspend,
    SYS_resume,
    SYS_open,
    SYS_close,
    SYS_read = 15,
    SYS_write,
    SYS_lseek,
    SYS_stat,
    SYS_fsync,
    SYS_dup = 20,
    SYS_getdirent,
    SYS_unlink,
    SYS_chdir,
    SYS_fstatfs,
    SYS_mount = 25,
    SYS_umount,
    SYS_getcwd,
    SYS_rename,
    SYS_brk,

/************** 以下内容不可添加与修改****************/
    SYS_callmax,
};



#endif
