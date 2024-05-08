
#include "defs.h"
#include "ringbuff.h"


void kRingbuf_init (ringbuf_t *rb, char *buf, int len)
{
    if (buf == NULL)
        kError(eSVC_Ringbuf, E_PARAM);
    if (len == 0)
        kError(eSVC_Ringbuf, E_PARAM);

    rb->buf = buf;

    rb->rIndex = 0;
    rb->wIndex = 0;

    rb->baseSize = len;
    rb->idleSize = len;
}
void kRingbuf_clean (ringbuf_t *rb)
{
    if (rb == NULL)
        kError (eSVC_Ringbuf, E_PARAM);
    
    memset(rb->buf, 0, rb->baseSize);
    rb->idleSize = rb->baseSize;
    rb->rIndex = 0;
    rb->wIndex = 0;
}

int kRingbuf_put (ringbuf_t *rb, char *buf, int len)
{
    int margin;

    if ((buf == NULL) || (rb == NULL))
        kError(eSVC_Ringbuf, E_PARAM);
    if ((len == 0) || (rb->idleSize == 0))
        return 0;

    if (rb->idleSize < len)
        len = rb->idleSize;
    margin = rb->baseSize - rb->wIndex;

    if (margin > len)
    {
        memcpy(&rb->buf[rb->wIndex], buf, len);
        rb->wIndex += len;
    }
    else
    {
        memcpy(&rb->buf[rb->wIndex], buf, margin);
        memcpy(&rb->buf[0], &buf[margin], len - margin);
        rb->wIndex = len - margin;
    }
    rb->idleSize -= len;

    return len;
}

int kRingbuf_get (ringbuf_t *rb, char *buf, int len)
{
    int margin;

    if ((rb == NULL) || (buf == NULL))
        kError(eSVC_Ringbuf, E_PARAM);
    if ((len == 0) || (rb->idleSize == rb->baseSize))
        return 0;

    if (len > (rb->baseSize - rb->idleSize))
        len = rb->baseSize - rb->idleSize;
    margin = rb->baseSize - rb->rIndex;

    if (margin > len)
    {
        memcpy(buf, &rb->buf[rb->rIndex], len);
        rb->rIndex += len;
    }
    else
    {
        memcpy(&buf[0], &rb->buf[rb->rIndex], margin);
        memcpy(&buf[margin], &rb->buf[0], len - margin);
        rb->rIndex = len - margin;
    }
    rb->idleSize += len;
    
    return len;
}

int kRingbuf_putchar (ringbuf_t *rb, char ch)
{
    if (rb == NULL)
        kError(eSVC_Ringbuf, E_PARAM);
    if (rb->idleSize == 0)
        return 0;

    rb->buf[rb->wIndex] = ch;
    rb->idleSize -= 1;

    if ((rb->baseSize - rb->wIndex) == 1)
    {
        rb->wIndex = 0;
    }
    else
    {
        rb->wIndex += 1;
    }

    return 1;
}

int kRingbuf_getchar (ringbuf_t *rb, char *ch)
{
    if (rb == NULL)
        kError(eSVC_Ringbuf, E_PARAM);
    if (rb->idleSize == rb->baseSize)
        return 0;

    *ch = rb->buf[rb->rIndex];
    rb->idleSize += 1;

    if ((rb->baseSize - rb->rIndex) == 1)
    {
        rb->rIndex = 0;
    }
    else
    {
        rb->rIndex += 1;
    }

    return 1;
}

