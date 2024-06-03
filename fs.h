
#ifndef __FS_H__
#define __FS_H__

#include "proc.h"


/* 记录用户可以读取到的文件系统信息
 * ( 虚拟文件系统对外接口使用的文件系统信息结构体 )
 */
struct statfs
{
    unsigned int f_bsize;  /* block size */
    unsigned int f_block; /* total data blocks in file system */
    unsigned int f_bfree;  /* free blocks in file system */
};


/* 记录用户可以读取到的文件信息 
 * ( 虚拟文件系统对外接口使用的文件信息结构体 )
 */
struct stat
{
    int     dev;
    int     size;
    int     mode;
};

#define SEEK_SET	0	/* 设置值为距离文件开始位置的绝对值 */
#define SEEK_CUR	1	/* 在当前位置修改偏移值 */
#define SEEK_END	2	/* 移动到文件末尾 */
#define SEEK_DATA	3	/* seek to the next data */
#define SEEK_HOLE	4	/* seek to the next hole */
#define SEEK_MAX	SEEK_HOLE


int  vfs_pcbInit   (ProcCB *pcb, char *path);
int  vfs_pcbdeinit (ProcCB *pcb);

void init_vfs   (void);
int  vfs_open   (char *path, unsigned int flags, unsigned int mode);
int  vfs_close  (int fd);
int  vfs_write  (int fd, void *buf, int len);
int  vfs_read   (int fd, void *buf, int len);
int  vfs_mount  (char *fsname, char *path, unsigned int flag, void *data);
int  vfs_unlink (char *path);
int  vfs_fsync  (int fd);
int  vfs_fstatfs(int fd, struct statfs *buf);
int  vfs_creat  (char *path, unsigned int mode);
int  vfs_rename (char *oldfile, char *newfile);

#endif
