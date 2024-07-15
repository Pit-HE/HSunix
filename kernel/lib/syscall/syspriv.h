
#ifndef __SYS_PRIVAT_H__
#define __SYS_PRIVAT_H__


#include "defs.h"


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
uint64 sys_brk (void);

/******************* sysfile **********************/
uint64 sys_open (void);
uint64 sys_close (void);
uint64 sys_read (void);
uint64 sys_write (void);
uint64 sys_lseek (void);
uint64 sys_stat (void);
uint64 sys_fstatfs (void);
uint64 sys_fsync (void);
uint64 sys_dup (void);
uint64 sys_getdirent(void);
uint64 sys_unlink (void);
uint64 sys_chdir (void);
uint64 sys_mount (void);
uint64 sys_umount(void);
uint64 sys_getcwd(void);
uint64 sys_rename (void);
uint64 sys_exec (void);
uint64 sys_pipe (void);


#endif
