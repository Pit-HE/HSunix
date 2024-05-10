
#ifndef __RING_BUFF_H__
#define __RING_BUFF_H__



typedef struct kernel_ringbuff_info
{
    char *buf;
    int baseSize;   // buf size
    int idleSize;   // idle buf size
    int rIndex;
    int wIndex;
}ringbuf_t;



#endif
