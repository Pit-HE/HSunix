
#ifndef __FCNTL_H__
#define __FCNTL_H__



/* 文件的操作，用于虚拟文件系统的 flag */
#define O_ACCMODE	0x00000003  /* 可访问掩码 */
#define O_RDONLY	0x00000000  /* 只读 */
#define O_WRONLY	0x00000001  /* 只写 */
#define O_RDWR		0x00000002  /* 可读可写 */
#define O_CREAT		0x00000100  /* 创建 */
#define O_EXCL		0x00000200
#define O_NOCTTY	0x00000400  /*  */
#define O_TRUNC		0x00001000  /* 丢弃文件内容 */
#define O_APPEND	0x00002000  /* 在文件末尾继续写入 */
#define O_DIRECTORY 0x00004000  /* 操作目录项 */

/* 文件的权限, 用于虚拟文件系统的 mode */
#define S_IRWXU     0x00000700  /* 用户可读可写可执行 */
#define S_IRUSR     0x00000400  /* 用户读 */
#define S_IWUSR     0x00000200  /* 用户写 */
#define S_IXUSR     0x00000100  /* 用户执行 */

/* 标准文件 */
#define STD_INPUT   0
#define STD_OUTPUT  1
#define STD_ERROR   2

#endif
