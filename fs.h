
#ifndef __FS_H__
#define __FS_H__


int fs_open    (const char *path, int flags);
int fs_close   (int fd);
int fs_write   (int fd, void *buf, int len);
int fs_read    (int fd, void *buf, int len);



#endif
