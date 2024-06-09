
#ifndef __DFS_RAMFS_H__
#define __DFS_RAMFS_H__


#include "list.h"

#define RAMFS_PATH_MAT  256    /* 最大的文件路径长度 */
#define RAMFS_NAME_LEN  32     /* 最大的文件名长度 */
#define RAMFS_MAGIC     0xFF05 /* 魔幻数的值 */


/* 文件或目录的节点 */
struct ramfs_node
{
    enum {
        RAMFS_FILE, RAMFS_DIR
    }type;                    /* 节点类型：文件/目录 */
    char             name[RAMFS_NAME_LEN];
    ListEntry_t      siblist; /* 链接同级节点的链表 */
    ListEntry_t      sublist; /* 链接子级节点的链表 */
    uint             flags;   /* 对应 inode 的 flags */
    uint             mode;    /* 对应 inode 的 mode */
    char            *data;    /* 文件存储数据的内存空间 */
    uint             size;    /* 文件的大小 */
    struct ramfs_sb *sb;      /* 所属的 ramfs */
};


/* 文件系统的超级块 */
struct ramfs_sb
{
    uint                magic;  /* 魔幻数 */
    struct ramfs_node   root;   /* 根目录节点 */
    uint                size;   /* 记录当前文件系统占用的总内存 */
    uint                flag;   /* 标记系统的访问方式(可读、可写...) */
    ListEntry_t         siblist;/* 记录根目录下的 node */
};


/* 文件系统唯一对外的初始化接口 */
void init_ramfs (void);

#endif
