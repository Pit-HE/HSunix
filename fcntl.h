
#ifndef __FCNTL_H__
#define __FCNTL_H__



#define O_ACCMODE	00000003
#define O_RDONLY	00000000
#define O_WRONLY	00000001
#define O_RDWR		00000002
#define O_CREAT		00000100
#define O_EXCL		00000200
#define O_NOCTTY	00000400
#define O_TRUNC		00001000    /* 丢弃文件内容 */
#define O_APPEND	00002000    /* 在文件末尾继续写入 */



#endif
