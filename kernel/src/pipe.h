
#ifndef __PIPE_H__
#define __PIPE_H__


#include "ringbuff.h"


#define PIPE_BUF_SIZE   16


struct pipe_t
{
    /* 管道读标志 */
    bool rflag;
    /* 管道写标志 */
    bool wflag;
    /* 管道数据存储缓冲区 */
    char *buf;
    /* 管理缓冲区的循环队列 */
    ringbuf_t rb;
};


#endif
