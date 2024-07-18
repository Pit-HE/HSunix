/*
 * 记录用户空间可以直接调用的内核接口
 */
#ifndef __USER_LIB_H__
#define __USER_LIB_H__


#include "string.h"


void    exit    (int code);
int     fork    (void);
int     wait    (int *code);
void    exec    (char *path, char *argv[]);
void    yield   (void);
void    kill    (int pid);
int     getpid  (void);
void    putc    (int ch);
int     getc    (void);
int     gettime (void);
void    sleep   (int ms);
int     open    (char *path, uint flags, uint mode);
int     close   (int fd);
int     read    (int fd, void *buf, int len);
int     write   (int fd, void *buf, int len);
int     lseek   (int fd, uint off, int whence);
int     stat    (char *path);
int     fstatfs (int fd, void *buf);
int     fsync   (int fd);
void    dup     (void);
void    suspend (void *obj);
void    resume  (void *obj);
int     getdirent (int fd, void *buf, uint len);
int     unlink  (char *path);
int     chdir   (char *path);
int     mount   (char *fsname, char *path);
int     umount  (char *path);
int     getcwd  (char *buf, int len);
int     rename  (char *oldname, char *newname);
int     brk     (void);
int     pipe    (int fd[2]);
int     msgget  (int key, int msgflg);
int     msgsnd  (int msqid, void *msgp, uint msgsz, int msgflg);
int     msgrcv  (int msqid, void *msgp, uint msgsz, int msgtyp, int msgflg);
int     msgctl  (int msqid, int cmd, void *uptr);

#include "uprintf.h"
#define printf tfp_printf


#include "shell.h"
#include "ipc.h"


#include "fcntl.h"
#include "dirent.h"
DIR *opendir(char *path);
int  closedir(DIR *dir);
void seekdir(DIR *dir, long offset);
int  rmdir (char *path);
struct dirent *readdir(DIR *dir);



/**************** memory ****************/
void *malloc (int size);
void free (void *obj);
void init_memory (void);


/**************** memory ****************/
#include "pthread.h"
/* 外部接口 */
int  pthread_exit   (void *retval);
int  pthread_join   (pthread_t thread, void **retval);
int  pthread_sleep  (int ms);
int  pthread_create (pthread_t *thread, const pthread_attr_t *attr, 
        void *(*start_routine)(void *), void *arg);


#endif
