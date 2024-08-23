
#ifndef __KSTRING_H__
#define __KSTRING_H__


#include "types.h"


void *kmemset (void *s, int c, uint n);
void *kmemmove(void *dst, const void *src, uint n);
void *kmemcpy (void *dst, const void *src, uint n);
int   kstrlen (const char *st);
char *kstrcpy (char *dest, const char *src);
int   kstrcmp (const char *p1, const char *p2);
int   kstrncmp(const char *p, const char *q, uint n);
char *kstrdup (const char *s);
char *kstrcat (char *dest, const char *src);
char *kstrchr (const char *str, int chr);
char *kstrrchr(const char *str, int ch);


#endif
