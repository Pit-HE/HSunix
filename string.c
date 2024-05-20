

#include "defs.h"


void *kmemset(void *s, int c, uint n)
{
    char *xs = s;
    while (n--)
        *xs++ = c;
    return s;
}

void *kmemmove(void *dst, const void *src, uint n)
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

void *kmemcpy (void *dst, const void *src, uint n)
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

int kstrlen (const char *st)
{
    int i;
    for (i=0; st[i]; i++)
    {}
    return i;
}

char *kstrcpy (char *dest, const char *src)
{
    char *ret = dest;
    while(*src != '\0')
        *dest++ = *src++;
    *dest = *src;
    return ret;
}

/* 字符串比较函数
 *
 * 返回值：0表示字符串完全相对
 */
int kstrcmp (const char *p1, const char *p2)
{
	const unsigned char *s1 = (const unsigned char *) p1;
    const unsigned char *s2 = (const unsigned char *) p2;
    unsigned char c1, c2;

    do {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        if(c1 == '\0')
            return c1 - c2;
    } while (c1 == c2);

    return c1 - c2;
}

int kstrncmp(const char *p, const char *q, uint n)
{
  while (n > 0 && *p && *p == *q)
    n--, p++, q++;
  if (n == 0)
    return 0;
  return (uchar)*p - (uchar)*q;
}

/* 拷贝字符串 */
char *kstrdup(const char *s)
{
    unsigned int len = kstrlen(s) + 1;
    char *tmp = (char *)kalloc(len);

    if (tmp == NULL)
        return NULL;
    kmemcpy(tmp, s, len);

    return tmp;
}