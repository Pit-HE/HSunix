
#include "types.h"


void *memset(void *s, int c, uint n)
{
    char *xs = s;
    while (n--)
        *xs++ = c;
    return s;
}

int strlen (const char *st)
{
    int i;
    for (i=0; st[i]; i++)
    {}
    return i;
}
