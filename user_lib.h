
#ifndef __USER_LIB_H__
#define __USER_LIB_H__



void    exit    (int code);
void    fork    (void);
void    wait    (int *code);
void    exec    (void);
void    yield   (void);
void    kill    (int pid);
int     getpid  (void);
void    putc    (int ch);
void    pgdir   (void);
int     gettime (void);
void    sleep   (int ms);
void    open    (void);
void    close   (void);
void    read    (void);
void    write   (void);
void    seek    (void);
void    fstat   (void);
void    fsync   (void);
void    dup     (void);
void    printf (char *fmt, ...);



#endif
