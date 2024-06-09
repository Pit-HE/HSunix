
#ifndef __RING_BUFF_H__
#define __RING_BUFF_H__



typedef struct kernel_ringbuff_info
{
    char *buf;      // 缓冲器地址
    int baseSize;   // 缓冲区本身的大小
    int idleSize;   // 当前缓冲区空闲的大小
    int rIndex;     // 读位置索引
    int wIndex;     // 写位置索引
}ringbuf_t;



#endif
