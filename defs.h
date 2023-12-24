
#ifndef __UCORE_DEFS_H__
#define __UCORE_DEFS_H__





/** uart **/
void uartinit(void);
void uartputc_async(int);
void uartputc_sync(int);
int  uartgetc_loop(void);
int  uartgetc_intr(void);
void uartintr (void);

/** console **/
int console_write (char *src, int n);
int console_read (char *src);
void console_init (void);

/** string **/
int strlen (const char *st);
void *memset(void *s, int c, uint n);



#endif
