/*
 * 记录所有的系统调用编号
 */
#ifndef __SYSTEM_CALL_H__
#define __SYSTEM_CALL_H__


enum system_call_code
{
    SYS_exit = 0,
    SYS_fork,
    SYS_wait,
    SYS_exec,
    SYS_clone,
    SYS_yield,
    SYS_sleep,
    SYS_suspend,
    SYS_resume,
    SYS_kill,
    SYS_gettime,
    SYS_getpid,
    SYS_mmap,
    SYS_munmap,
    SYS_shmem,
    SYS_putc,
    SYS_getc,
    SYS_pgdir,
    SYS_open,
    SYS_close,
    SYS_read,
    SYS_write,
    SYS_lseek,
    SYS_fstat,
    SYS_fsync,
    SYS_dup,

/************** 以下内容不可添加与修改****************/
    SYS_callmax,
};


#endif
