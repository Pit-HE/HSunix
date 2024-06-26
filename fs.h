
#ifndef __FS_H__
#define __FS_H__

#include "proc.h"


/* 记录用户可以读取到的文件系统信息
 * ( 虚拟文件系统对外接口使用的文件系统信息结构体 )
 */
struct statfs
{
    char name[FS_NAME_LEN];
    uint f_total;  /* 文件系统的总大小 */
    uint f_bsize;  /* block size */
    uint f_block;  /* total data blocks in file system */
    uint f_bfree;  /* free blocks in file system */
};


/* 记录用户可以读取到的文件信息 
 * ( 虚拟文件系统对外接口使用的文件信息结构体 )
 */
struct stat
{
    int  size;      /* 对象的大小 */
    int  type;      /* 文件对象的类型 */
    char name[32];  /* 文件对象的名字 */
    /* 文件对象所属的文件系统 */
    char fsname[FS_NAME_LEN];
};

#define SEEK_SET	0	/* 设置值为距离文件开始位置的绝对值 */
#define SEEK_CUR	1	/* 在当前位置修改偏移值 */
#define SEEK_END	2	/* 移动到文件末尾 */
#define SEEK_DATA	3	/* seek to the next data */
#define SEEK_HOLE	4	/* seek to the next hole */
#define SEEK_MAX	SEEK_HOLE

#define VFS_DIR     1
#define VFS_FILE    2


int  vfs_pcbInit   (struct ProcCB *pcb, char *path);
int  vfs_pcbdeinit (struct ProcCB *pcb);

void init_vfs   (void);
int  vfs_open   (char *path, uint flags, uint mode);
int  vfs_close  (int fd);
int  vfs_write  (int fd, void *buf, int len);
int  vfs_read   (int fd, void *buf, int len);
int  vfs_mount  (char *fsname, char *path, uint flag, void *data);
int  vfs_unmount(char *path);
int  vfs_unlink (char *path);
int  vfs_fsync  (int fd);
int  vfs_fstatfs(int fd, struct statfs *buf);
int  vfs_creat  (char *path, uint mode);
int  vfs_rename (char *oldname, char *newname);
int  vfs_stat   (int fd, struct stat *buf);
int  vfs_lseek  (int fd, uint off, int whence);

#endif
