/*
 * 存放磁盘文件系统关于具体磁盘块的操作
 * 1、依赖于 dfs_iobuf 层的支持去操作磁盘
 * 2、主要处理软件概念上划分的不同磁盘块
 */
#include "defs.h"
#include "dfs_priv.h"


/**************** 对磁盘超级块的操作 *****************/
#if 1
/* 获取磁盘中记录的超级块信息 */
struct disk_sb *dsb_read (void)
{
    struct Iobuf *buf = NULL;
	struct disk_sb *sb;

	sb = (struct disk_sb *)kalloc(sizeof(struct disk_sb));
	if (sb == NULL)
		return NULL;

    /* 读取超级块所在的磁盘块 */
    buf = iob_alloc(1);
	if (buf == NULL)
		return NULL;

    /* 获取超级块的信息 */
    kmemmove(sb, buf->data, sizeof(struct disk_sb));

	iob_free(buf);

    return sb;
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
/* 将磁盘节点在内存中的信息，写回到磁盘中 */
void dnode_flush (struct disk_sb *sb, struct dinode *dnode)
{
	struct Iobuf *buf = NULL;
	struct dinode *node = NULL;

	/* 获取内存 inode 对应的磁盘 dinode 所在的磁盘块 */
	buf = iob_alloc(IBLOCK(dnode->nlink, sb));
	node = (struct dinode *)buf->data + (dnode->nlink %
		(BSIZE / sizeof(struct dinode)));

	/* 将内存 inode 信息写入磁盘 dinode */
	kmemmove(node, dnode, sizeof(struct dinode));

	/* 将磁盘块写回到日志系统 */
	iob_flush(buf);
}

/* 获取一个空闲磁盘索引节点的编号 */
int dnode_get (struct disk_sb *sb, uint type)
{
	int inum;
	struct Iobuf *buf = NULL;
	struct dinode *node = NULL;

	/* 遍历超级块上的所有 dinode */
	for(inum = 1; inum < sb->ninodes; inum++)
	{
		/* 遍历索引节点磁盘块上记录的所有 dinode 信息 */
		buf = iob_alloc(IBLOCK(inum, sb));
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

			/* mark it allocated on the disk */
			iob_flush(buf);
			iob_free(buf);

			/* 获取与 dinode 对应的内存 inode */
			return inum;
		}
		iob_free(buf);
	}
	kprintf("ialloc: no inodes\n");
	return -1;
}

/* 将指定的磁盘节点写回到磁盘中 */
void dnode_put (struct disk_sb *sb, struct dinode *dnode)
{
	struct Iobuf *buf = NULL;
	struct dinode *node = NULL;

	/* 获取内存 inode 对应的磁盘 dinode 所在的磁盘块 */
	buf = iob_alloc(IBLOCK(dnode->nlink, sb));
	node = (struct dinode *)buf->data + (dnode->nlink %
		(BSIZE / sizeof(struct dinode)));

	/* 将内存 inode 信息写入磁盘 dinode */
	kmemmove(node, dnode, sizeof(struct dinode));

	/* 将磁盘块写回到日志系统 */
	iob_flush(buf);
	iob_free(buf);
}

/* 获取存放 dinode 的内存空间，并读取对应的磁盘信息 */
struct dinode *dnode_alloc (struct disk_sb *sb, uint inum)
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
	buf = iob_alloc(IBLOCK(inum, sb));
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
void dnode_free (struct disk_sb *sb, struct dinode *dnode)
{
	if (dnode == NULL)
		return;

	/* 将磁盘节点的信息写回磁盘 */
	dnode_put(sb, dnode);
	/* 释放磁盘节点占用的内存空间 */
	kfree(dnode);
}

#endif

/**************** 对磁盘位图块的操作 *****************/
#if 1
/* 获取当前空闲磁盘块的编号 */
uint dbmap_alloc (struct disk_sb *sb)
{
    int blknum, bi, map;
	struct Iobuf *buf = NULL;

	/* 遍历超级块中记录的所有磁盘块 */
	for(blknum = 0; blknum < sb->size; blknum += BPB)
	{
		/* 获取该块对象对应的位图所在的磁盘块 */
		buf = iob_alloc(BBLOCK(blknum, sb));

		/* 遍历该位图磁盘块上的每一个块 */
		for(bi = 0; bi < BPB && blknum + bi < sb->size; bi++)
		{
			map = 1 << (bi % 8);

			/* 获取一个空闲的磁盘块 */
			if((buf->data[bi / 8] & map) == 0)
			{
				/* 置起该磁盘块的标准为已分配 */
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
void dbmap_free (struct disk_sb *sb, uint blknum)
{
    struct Iobuf * buf;
	int bi, map;

	/* 获取该块对象对应位图所在的磁盘块 */
	buf = iob_alloc(BBLOCK(blknum, sb));

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
