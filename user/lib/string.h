
#ifndef __STRING_H__
#define __STRING_H__


#include "types.h"


void *memset (void *s, int c, uint n);
void *memmove(void *dst, const void *src, uint n);
void *memcpy (void *dst, const void *src, uint n);
int   strlen (const char *st);
char *strcpy (char *dest, const char *src);
int   strcmp (const char *p1, const char *p2);
int   strncmp(const char *p, const char *q, uint n);
char *strcat (char *dest, const char *src);
char *strchr (const char *str, int chr);
char *strrchr(const char * str, int ch);



#endif
