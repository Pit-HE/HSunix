
#include "types.h"


void *memset(void *s, int c, uint n)
{
    char *xs = s;
    while (n--)
        *xs++ = c;
    return s;
}

void *memmove(void *dst, const void *src, uint n)
{
    char *tmp = (char *)dst, *s = (char *)src;

    if (s < tmp && tmp < s + n)
    {
        tmp += n;
        s += n;

        while (n--)
            *(--tmp) = *(--s);
    }
    else
    {
        while (n--)
            *tmp++ = *s++;
    }

    return dst;
}

void *memcpy (void *dst, const void *src, uint n)
{
    char *tmp = (char *)dst, *s = (char *)src;
    uint len = 0;

    if (tmp <= s || tmp > (s + n))
    {
        while (n--)
            *tmp ++ = *s ++;
    }
    else
    {
        for (len = n; len > 0; len --)
            tmp[len - 1] = s[len - 1];
    }

    return dst;
}

int strlen (const char *st)
{
    int i;
    for (i=0; st[i]; i++)
    {}
    return i;
}

char *strcpy (char *dest, const char *src)
{
    char *ret = dest;
    while(*src != '\0')
        *dest++ = *src++;
    *dest = *src;
    return ret;
}
