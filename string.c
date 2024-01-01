
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
  const char *s;
  char *d;

  if (n == 0)
    return dst;

  s = src;
  d = dst;
  if (s < d && s + n > d)
  {
    s += n;
    d += n;
    while (n-- > 0)
      *--d = *--s;
  }
  else
    while (n-- > 0)
      *d++ = *s++;

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
