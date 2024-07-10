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
    SYS_getdirent,
    SYS_unlink,
    SYS_chdir,

/************** 以下内容不可添加与修改****************/
    SYS_callmax,
};


/******************* syscall **********************/
void arg_int (int num, int *data);
void arg_addr(int num, uint64 *addr);
int  arg_str (int num, char *buf, int len);

/******************* sysproc **********************/
uint64 sys_exit (void);
uint64 sys_fork (void);
uint64 sys_wait (void);
uint64 sys_yield (void);
uint64 sys_kill (void);
uint64 sys_getpid (void);
uint64 sys_putc (void);
uint64 sys_getc (void);
uint64 sys_suspend (void);
uint64 sys_resume (void);
uint64 sys_gettime (void);
uint64 sys_sleep (void);

/******************* sysfile **********************/
uint64 sys_pgdir (void);
uint64 sys_open (void);
uint64 sys_close (void);
uint64 sys_read (void);
uint64 sys_write (void);
uint64 sys_lseek (void);
uint64 sys_fstat (void);
uint64 sys_fsync (void);
uint64 sys_dup (void);
uint64 sys_getdirent(void);
uint64 sys_unlink (void);
uint64 sys_chdir (void);
uint64 sys_exec (void);


#endif
