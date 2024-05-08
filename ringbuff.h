
#ifndef __RING_BUFF_H__
#define __RING_BUFF_H__



typedef struct kernel_ringbuff_info
{
    char *buf;
    int baseSize;
    int idleSize;
    int rIndex;
    int wIndex;
}ringbuf_t;



#endif
