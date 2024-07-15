
#include "defs.h"
#include "ringbuff.h"


/* 初始化缓冲区对象 */
void kRingbuf_init (ringbuf_t *rb, char *buf, int len)
{
    if ((buf == NULL) || (len == 0))
        ErrPrint("kRingbuf_init: Illegal function argument !\r\n");

    kmemset(rb, 0, sizeof(ringbuf_t));

    rb->buf = buf;
    rb->baseSize = len;

    kRingbuf_clean(rb);
}

/* 清空缓冲区内记录的数据 */
void kRingbuf_clean (ringbuf_t *rb)
{
    if (rb == NULL)
        ErrPrint("kRingbuf_clean: Illegal function argument !\r\n");

    kmemset(rb->buf, 0, rb->baseSize);
    rb->idleSize = rb->baseSize;
    rb->rIndex = 0;
    rb->wIndex = 0;
}

/* 往缓冲区写入指定长度的数据 */
int kRingbuf_put (ringbuf_t *rb, char *buf, int len)
{
    int margin;

    if ((buf == NULL) || (rb == NULL))
        ErrPrint("kRingbuf_put: Illegal function argument !\r\n");
    if ((len == 0) || (rb->idleSize == 0))
        return 0;

    if (rb->idleSize < len)
        len = rb->idleSize;
    margin = rb->baseSize - rb->wIndex;

    if (margin > len)
    {
        /* 要读取的数据不涉及缓冲区的重新循环 */
        kmemcpy(&rb->buf[rb->wIndex], buf, len);
        rb->wIndex += len;
    }
    else
    {
        /* 要读取的数据涉及缓冲区的重新循环 */
        kmemcpy(&rb->buf[rb->wIndex], buf, margin);
        kmemcpy(&rb->buf[0], &buf[margin], len - margin);
        rb->wIndex = len - margin;
    }
    rb->idleSize -= len;

    return len;
}

/* 从缓冲区内获取指定长度的数据 */
int kRingbuf_get (ringbuf_t *rb, char *buf, int len)
{
    int margin;

    if ((rb == NULL) || (buf == NULL))
        ErrPrint("kRingbuf_get: Illegal function argument !\r\n");
    if ((len == 0) || (rb->idleSize == rb->baseSize))
        return 0;

    /* 计数缓冲区的数据长度，确定用户可以读取的数据长度 */
    if (len > (rb->baseSize - rb->idleSize))
        len = rb->baseSize - rb->idleSize;
    margin = rb->baseSize - rb->rIndex;

    if (margin > len)
    {
        /* 要读取的数据不涉及缓冲区的重新循环 */
        kmemcpy(buf, &rb->buf[rb->rIndex], len);
        rb->rIndex += len;
    }
    else
    {
        /* 要读取的数据涉及缓冲区的重新循环 */
        kmemcpy(&buf[0], &rb->buf[rb->rIndex], margin);
        kmemcpy(&buf[margin], &rb->buf[0], len - margin);
        rb->rIndex = len - margin;
    }
    rb->idleSize += len;

    return len;
}

/* 往缓冲区内写入单个字符 */
int kRingbuf_putChar (ringbuf_t *rb, char ch)
{
    if (rb == NULL)
        ErrPrint("kRingbuf_putChar: Illegal function argument !\r\n");
    if (rb->idleSize == 0)
        return 0;

    rb->buf[rb->wIndex] = ch;
    rb->idleSize -= 1;
    rb->wIndex += 1;

    /* 若是数组写到了头，则从数组头继续开始 */
    if (rb->baseSize == rb->wIndex)
        rb->wIndex = 0;

    return 1;
}

/* 从缓冲区内读取单个字符 */
int kRingbuf_getChar (ringbuf_t *rb, char *ch)
{
    if (rb == NULL)
        ErrPrint("kRingbuf_getChar: Illegal function argument !\r\n");
    if (rb->idleSize == rb->baseSize)
        return 0;

    *ch = rb->buf[rb->rIndex];
    rb->idleSize += 1;
    rb->rIndex += 1;

    /* 若是数组写到了头，则从数组头继续开始 */
    if (rb->baseSize == rb->rIndex)
        rb->rIndex = 0;

    return 1;
}

/* 删除上一次接收到的字符 */
int kRingbuf_delChar (ringbuf_t *rb)
{
    if (rb == NULL)
        ErrPrint("kRingbuf_getChar: Illegal function argument !\r\n");
    if (rb->idleSize >= rb->baseSize)
        return -1;
    if (rb->rIndex == rb->wIndex)
        return -1;

    /* 回退写指针的位置 */
    if (rb->wIndex == 0)
        rb->wIndex = rb->baseSize;
    else
        rb->wIndex -= 1;

    /* 修改循环队列的信息 */
    rb->idleSize += 1; 
    rb->buf[rb->wIndex] = 0;

    return 1;
}

/* 确认缓冲区是否还可以继续写数据 */
int kRingbuf_putState (ringbuf_t *rb)
{
    if (rb->idleSize == 0)
        return 0;
    return 1;
}

/* 确认缓冲区是否还可以继续读数据 */
int kRingbuf_getState (ringbuf_t *rb)
{
    if (rb->idleSize == rb->baseSize)
        return 0;
    return 1;
}

/* 获取缓冲区内存放的数据长度 */
int kRingbuf_getLength (ringbuf_t *rb)
{
    int len;

    kDISABLE_INTERRUPT();
    len = rb->baseSize - rb->idleSize;
    kENABLE_INTERRUPT();

    return len;
}

/* 获取缓冲区内可以读取的数据长度 */
int KRingbuf_rDataSize (ringbuf_t *rb)
{
    return (rb->baseSize - rb->idleSize);
}

/* 获取缓冲区内可以写入的数据长度 */
int KRingbuf_wDataSize (ringbuf_t *rb)
{
    return (rb->idleSize);
}
