
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


void kRingbuf_init      (ringbuf_t *rb, char *buf, int len);
void kRingbuf_clean     (ringbuf_t *rb);
int  kRingbuf_put       (ringbuf_t *rb, char *buf, int len);
int  kRingbuf_get       (ringbuf_t *rb, char *buf, int len);
int  kRingbuf_putChar   (ringbuf_t *rb, char  ch);
int  kRingbuf_getChar   (ringbuf_t *rb, char *ch);
int  kRingbuf_delChar   (ringbuf_t *rb);
int  kRingbuf_putState  (ringbuf_t *rb);
int  kRingbuf_getState  (ringbuf_t *rb);
int  kRingbuf_getLength (ringbuf_t *rb);
int  KRingbuf_rDataSize (ringbuf_t *rb);
int  KRingbuf_wDataSize (ringbuf_t *rb);


#endif
