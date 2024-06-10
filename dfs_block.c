/*
 * 存放磁盘文件系统关于具体磁盘块的操作
 * 1、依赖于 dfs_iobuf 层的支持去操作磁盘
 * 2、主要处理软件概念上划分的不同磁盘块
 */
#include "defs.h"
#include "dfs_priv.h"

struct disk_sb dd_sb;



/**************** 对磁盘超级块的操作 *****************/
#if 1
/* 获取磁盘中记录的超级块信息 */
struct disk_sb *dsb_read (void)
{
    struct Iobuf *buf = NULL;

    /* 读取超级块所在的磁盘块 */
    buf = iob_alloc(1);
    /* 获取超级块的信息 */
    kmemmove(&dd_sb, buf->data, sizeof(dd_sb));

	iob_free(buf);

    return &dd_sb;
}

/* 将内存中的超级块信息写入磁盘 */
void dsb_write (struct disk_sb *sb)
{
	struct Iobuf *buf = NULL;

    /* 读取超级块所在的磁盘块 */
    buf = iob_alloc(1);

	/* 将超级块的信息写入磁盘对象中 */
	kmemmove(buf->data, sb, sizeof(struct disk_sb));

	iob_flush(buf);
	iob_free(buf);
}

#endif

/************** 对磁盘索引节点块的操作 ***************/
#if 1
/* 获取一个空闲的磁盘索引节点 */
struct dinode *dinode_get (uint type)
{
	int inum;
	struct Iobuf *buf = NULL;
	struct dinode *node = NULL;
	struct dinode *dnode = NULL;

	dnode = (struct dinode *)kalloc(sizeof(struct dinode));
	if (dnode == NULL)
		return NULL;

	/* 遍历超级块上的所有 dinode */
	for(inum = 1; inum < dd_sb.ninodes; inum++)
	{
		/* 遍历索引节点磁盘块上记录的所有 dinode 信息 */
		buf = iob_alloc(IBLOCK(inum, dd_sb));
		node = (struct dinode *)buf->data + (inum % 
			(BSIZE / sizeof(struct dinode)));

		/* 寻找未分配的磁盘 dnode */
		if(node->type == 0)
		{
			/* 清空 dinode 的数据 */
			kmemset(node, 0, sizeof(struct dinode));

			/* 设置当前磁盘 dinode 的类型 */
			node->type = type;
			node->nlink = inum;

			dnode->type = type;
			dnode->nlink = inum;

			/* mark it allocated on the disk */
			iob_flush(buf);
			iob_free(buf);

			/* 获取与 dinode 对应的内存 inode */
			return dnode;
		}
		iob_free(buf);
	}
	kprintf("ialloc: no inodes\n");
	return NULL;
}

/* 将指定的磁盘节点写回到磁盘中 */
void dinode_put (struct dinode *dnode)
{
	struct Iobuf *buf = NULL;
	struct dinode *node = NULL;

	/* 获取内存 inode 对应的磁盘 dinode 所在的磁盘块 */
	buf = iob_alloc(IBLOCK(dnode->nlink, dd_sb));
	node = (struct dinode *)buf->data + (dnode->nlink %
		(BSIZE / sizeof(struct dinode)));

	/* 将内存 inode 信息写入磁盘 dinode */
	kmemmove(node, dnode, sizeof(struct dinode));

	/* 将磁盘块写回到日志系统 */
	iob_flush(buf);
	iob_free(buf);

	kfree(dnode);
}

/* 获取存放 dinode 的内存空间，并读取对应的磁盘信息 */
struct dinode *dinode_alloc (uint inum)
{
	struct Iobuf *buf = NULL;
	struct dinode *node = NULL;
	struct dinode *temp = NULL;

	/* 获取存放磁盘节点的空间 */
    node = (struct dinode *)kalloc(sizeof(struct dinode));
	if (node == NULL)
		return NULL;

	/* 是否需要读取磁盘内记录的索引节点的信息 */
	if (inum == 0)
		return node;

	/* 获取磁盘节点所在的磁盘块 */
	buf = iob_alloc(IBLOCK(inum, dd_sb));
	if (buf == NULL)
		return NULL;
	
	/* 获取磁盘节点具体的位置 */
	temp = (struct dinode *)buf->data + inum % (BSIZE / sizeof(struct dinode));

	/* 将磁盘节点的信息拷贝到内存对象中 */
	kmemcpy(node, temp, sizeof(struct dinode));
	iob_free(buf);

	return node;
}

/* 释放一块存放磁盘 dinode 信息的内存空间 */
void dinode_free (struct dinode *dnode)
{
	if (dnode == NULL)
		return;

	/* 将磁盘节点的信息写回磁盘 */
	dinode_put(dnode);
	/* 释放磁盘节点占用的内存空间 */
	kfree(dnode);
}

#endif

/**************** 对磁盘位图块的操作 *****************/
#if 1
/* 获取当前空闲磁盘块的编号 */
uint dbmap_alloc (void)
{
    int blknum, bi, map;
	struct Iobuf *buf = NULL;

	/* 遍历超级块中记录的所有磁盘块 */
	for(blknum = 0; blknum < dd_sb.size; blknum += BPB)
	{
		/* 获取该块对象对应的位图所在的磁盘块 */
		buf = iob_alloc(BBLOCK(blknum, dd_sb));

		/* 遍历该位图磁盘块上的每一个块 */
		for(bi = 0; bi < BPB && blknum + bi < dd_sb.size; bi++)
		{
			map = 1 << (bi % 8);

			/* 获取一个空闲的磁盘块 ----- Is block free? */
			if((buf->data[bi / 8] & map) == 0)
			{
				// Mark block in use.
				buf->data[bi / 8] |= map;

				/* 将位图所在的磁盘块写回磁盘 */
                iob_flush(buf);

				/* 释放占用的 buf */
                iob_free(buf);

				/* 获取空闲的磁盘块，并将其清零 */
				dblk_zero(blknum + bi);

				/* 返回磁盘块的编号 */
				return blknum + bi;
			}
		}
		/* 释放占用的 buf */
		iob_free(buf);
	}
	kprintf("balloc: out of blocks\n");
	return 0;
}

/* 释放已经获取的磁盘块 */
void dbmap_free (uint blknum)
{
    struct Iobuf * buf;
	int bi, map;

	/* 获取该块对象对应位图所在的磁盘块 */
	buf = iob_alloc(BBLOCK(blknum, dd_sb));

	/* 清除磁盘块在位图中的已分配记录 */
	bi = blknum % BPB;
	map = 1 << (bi % 8);
	if((buf->data[bi / 8] & map) == 0)
    {
        kError(eSVC_fs, E_STATUS);
    }
	buf->data[bi / 8] &= ~map;

	/* 将位图所在的块写回磁盘 */
    iob_flush(buf);
    iob_free(buf);
}

#endif

/**************** 对磁盘块的操作 *****************/
#if 1

/* 将指定的磁盘块内容清空为零 */
void dblk_zero (uint blknum)
{
    struct Iobuf *buf = NULL;

    /* 将该磁盘块内容清空 */
    buf = iob_alloc(blknum);
    if (buf != NULL)
        kmemset(buf->data, 0, BSIZE);
}

/* 将数据写入磁盘块中 */
uint dblk_write (uint blknum, char *data, uint len)
{
	struct Iobuf *buf = NULL;

	buf = iob_alloc(blknum);

	if (len > BSIZE)
		len = BSIZE;

	kmemmove(&buf->data, data, len);

	iob_flush(buf);
	iob_free(buf);

	return len;
}

/* 从指定的磁盘块读取指定长度的数据 */ 
uint dblk_read (uint blknum, char *data, uint len)
{
	struct Iobuf *buf = NULL;

	buf = iob_alloc(blknum);

	if (len > BSIZE)
		len = BSIZE;
	
	kmemmove(data, &buf->data, len);

	return len;
}

#endif
