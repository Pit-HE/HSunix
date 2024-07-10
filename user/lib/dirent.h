
#ifndef __DIRENT_H__
#define __DIRENT_H__


#include "types.h"


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
    uint type;      /* 接收到的文件类型 */
    uint namelen;   /* 实体文件系统支持的文件名长度 */
    uint objsize;   /* 接收到的单个对象的长度 */
    uint datasize;  /* 当前这个文件的大小 */
    char name[128]; /* 目录项的名字 */
};


#define SEEK_SET	0	/* 设置值为距离文件开始位置的绝对值 */
#define SEEK_CUR	1	/* 在当前位置修改偏移值 */
#define SEEK_END	2	/* 移动到文件末尾 */
#define SEEK_DATA	3	/* seek to the next data */
#define SEEK_HOLE	4	/* seek to the next hole */
#define SEEK_MAX	SEEK_HOLE



#endif
