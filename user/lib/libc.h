/*
 * 记录用户空间可以直接调用的内核接口
 */
#ifndef __USER_LIB_H__
#define __USER_LIB_H__

#include "string.h"

void    exit    (int code);
int     fork    (void);
void    wait    (int *code);
void    exec    (char *path, char *argv[]);
void    yield   (void);
void    kill    (int pid);
int     getpid  (void);
void    putc    (int ch);
int     getc    (void);
void    pgdir   (void);
int     gettime (void);
void    sleep   (int ms);
int     open    (char *path, uint flags, uint mode);
int     close   (int fd);
int     read    (int fd, void *buf, int len);
int     write   (int fd, void *buf, int len);
int     lseek   (int fd, uint off, int whence);
void    fstat   (void);
int     fsync   (int fd);
void    dup     (void);
void    suspend (void *obj);
void    resume  (void *obj);

#include "uprintf.h"
#define printf tfp_printf

#include "shell.h"


#endif
