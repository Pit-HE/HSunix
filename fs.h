
#ifndef __FS_H__
#define __FS_H__


#include "defs.h"

int fs_open    (const char *file, int flags);
int fs_close   (int fd);
int fs_write   (int fd, void *buf, uint len);
int fs_read    (int fd, void *buf, uint len);



#endif
