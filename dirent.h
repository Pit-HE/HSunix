/*
 * 记录用户可用的关于文件目录的操作与信息
 */
#ifndef __DIRENT_H__
#define __DIRENT_H__

#include "param.h"

/* 用于目录操作中，缓存多个 dirent 的结构体
 * ( 减少操作 dirent 时读写文件系统的次数 )
 */
typedef struct
{
    int  fd;         /* directory file */
    char buf[512];   /* 存放 struct dirent 的信息 */
    int  bufsize;    /* 当前存放的 dirent 占用的 buf 空间 */
    int  index;      /* buf 中 dirent 对象的索引下标 */
}DIR;



/* 目录信息
 * ( 提供给用户层获取目录项信息的结构体 )
 */
struct dirent
{
    unsigned int type;      /* 接收到的文件类型 */
    unsigned int namelen;   /* 实体文件系统支持的文件名长度 */
    unsigned int objsize;   /* 接收到的单个对象的长度 */
    unsigned int datasize;  /* 当前这个文件的大小 */
    char name[128];         /* 目录项的名字 */
};



DIR *opendir(char *path);
int  closedir(DIR *dir);
void seekdir(DIR *dir, long offset);
long telldir(DIR *dir);
struct dirent *readdir(DIR *dir);
int mkdir  (char *path, unsigned int mode);
int mkfile (char *path, unsigned int flag, unsigned int mode);
int chdir(char *path);
int rmdir (char *path);

#endif
