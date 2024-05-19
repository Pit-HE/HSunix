
#ifndef __FS_H__
#define __FS_H__


/* 记录文件系统的信息 */
struct statfs
{
    unsigned int f_bsize;   /* block size */
    unsigned int f_blocks;  /* total data blocks in file system */
    unsigned int f_bfree;   /* free blocks in file system */
};

/* 记录文件或目录的信息 */
struct stat
{
    int     dev;
    int     size;
    int     mode;
};

int vfs_open  (const char *path, int flags);
int vfs_close (int fd);
int vfs_write (int fd, void *buf, int len);
int vfs_read  (int fd, void *buf, int len);



#endif
