/*
 * 磁盘数据缓冲模块，隔离磁盘读写与内存读写的差异
 */
#include "defs.h"
#include "file.h"
#include "fs_buf.h"

void buf_init (void)
{

}

struct Buf *buf_alloc (void)
{
    return NULL;
}
void buf_free (struct Buf *buf)
{

}
struct Buf *buf_read (uint dev, uint bNum)
{
    return NULL;
}
void buf_write (struct Buf *buf)
{

}



