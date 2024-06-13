/*
 * 存放磁盘文件系统关于具体磁盘块的操作
 * 1、依赖于 dfs_iobuf 层的支持去操作磁盘
 * 2、主要处理软件概念上划分的不同磁盘块
 */
#include "defs.h"
#include "dfs_priv.h"

struct disk_sb *superblock = NULL;
struct disk_inode *root_node = NULL;


/* 返回 inode 中第 bn 个数据块的磁盘块号 */
uint bmap (struct disk_inode *dnode, uint bn)
{
	uint addr, *tmp;
	struct disk_buf *buf;

	/* 处理 inode 磁盘数组空间足够的情况 */
	if(bn < NDIRECT)
	{
		/* 处理索引的数据块还未分配磁盘空间的情况 */
		if((addr = dnode->addrs[bn]) == 0)
		{
			/* 获取设备中的空闲磁盘块编号 */
			addr = dbmap_alloc();
			if(addr == 0)
				return 0;

			/* 记录分配到的空闲磁盘块编号 */
			dnode->addrs[bn] = addr;
		}
		return addr;
	}
	bn -= NDIRECT;

	/* 扩展 inode 的磁盘块建立更多磁盘块映射索引 */
	if(bn < NINDIRECT)
	{
		/* 处理最后一个数组成员还未分配磁盘空间的情况 */
		if((addr = dnode->ex_addr) == 0)
		{
			/* 获取空闲磁盘块编号 */
			addr = dbmap_alloc();
			if(addr == 0)
				return 0;

			/* 记录分配到的空闲磁盘块编号 */
			dnode->ex_addr = addr;
		}

		/* 读取磁盘块的内容 */
        buf = dbuf_alloc(addr);
		tmp = (uint *)buf->data;

		/* 将磁盘块格式化为一个个索引项，
		 并获取第一个空闲的索引项 */
		if((addr = tmp[bn]) == 0)
		{
			/* 获取空闲磁盘块的编号 */
			addr = dbmap_alloc();
			if(addr)
			{
				/* 将索引项与磁盘块建立联系 */
				tmp[bn] = addr;
				/* 将修改后的索引信息写回磁盘 */
                dbuf_flush(buf);
			}
		}
        dbuf_free(buf);
		return addr;
	}

	/* inode 可用空间溢出，不支持继续扩展 */
	kErrPrintf("bmap: out of range");
    return -1;
}

/* 获取文件路径中第一个节点的名字 */
char *disk_path_getfirst (char *path, char *name)
{
    char *str;
	int len;

	if ((path == NULL) || (name == NULL))
		return NULL;

	/* 跳过第一个路径分隔符 */
	while(*path == '/')
		path++;
	/* 确认剩下的路径不为空 */
	if(*path == 0)
		return 0;

	/* 解析路径中的第一个元素的名字 */
	str = path;
	while(*path != '/' && *path != 0)
		path++;
	len = path - str;

	/* 将元素的名字写入 name 参数 */
	if(len >= 14)
		kmemmove(name, str, 14);
	else
	{
		kmemmove(name, str, len);
		name[len] = 0;
	}
	/* 再次跳过文件分隔符 */
	while(*path == '/')
		path++;
	return path;
}

/* 获取文件路径中最后一个节点的名字 */
char *disk_path_getlast (char *path, char *name)
{
	path_getlast(path, NULL, name);
	return NULL;
}


/**************** 对磁盘超级块的操作 *****************/
#if 1
/* 获取磁盘中记录的超级块信息 */
struct disk_sb *dsb_get (void)
{
    struct disk_buf *buf = NULL;

	if (superblock != NULL)
		return superblock;

	superblock = (struct disk_sb *)kalloc(sizeof(struct disk_sb));
	if (superblock == NULL)
		return NULL;

    /* 读取超级块所在的磁盘块 */
    buf = dbuf_alloc(1);
	if (buf == NULL)
		return NULL;

    /* 获取超级块的信息 */
    kmemmove(superblock, buf->data, sizeof(struct disk_sb));

	dbuf_free(buf);

    return superblock;
}

/* 将内存中的超级块信息写入磁盘 */
void dsb_put (void)
{
	struct disk_buf *buf = NULL;

	if (superblock == NULL)
		return;

    /* 读取超级块所在的磁盘块 */
    buf = dbuf_alloc(1);

	/* 将超级块的信息写入磁盘对象中 */
	kmemmove(buf->data, superblock, sizeof(struct disk_sb));

	dbuf_flush(buf);
	dbuf_free(buf);
}

#endif

/**************** 对磁盘位图块的操作 *****************/
#if 1
/* 获取当前空闲磁盘块的编号 */
uint dbmap_alloc (void)
{
    int blknum, bi, map;
	struct disk_buf *buf = NULL;

	/* 遍历超级块中记录的所有磁盘块 */
	for(blknum = 0; blknum < superblock->size; blknum += (BSIZE * 8))
	{
		/* 获取该块对象对应的位图所在的磁盘块 */
		buf = dbuf_alloc(BBLOCK(blknum, superblock));

		/* 遍历该位图磁盘块上的每一个块 */
		for(bi = 0; bi < (BSIZE * 8) && blknum + bi < superblock->size; bi++)
		{
			map = 1 << (bi % 8);

			/* 获取一个空闲的磁盘块 */
			if((buf->data[bi / 8] & map) == 0)
			{
                /* 置起该磁盘块的标准为已分配 */
                buf->data[bi / 8] |= map;

                /* 将位图所在的磁盘块写回磁盘 */
                dbuf_flush(buf);

                /* 释放占用的 buf */
                dbuf_free(buf);

                /* 获取空闲的磁盘块，并将其清零 */
                dblk_zero(blknum + bi);

                /* 返回磁盘块的编号 */
                return blknum + bi;
			}
		}
		/* 释放占用的 buf */
		dbuf_free(buf);
	}
	kprintf("balloc: out of blocks\n");
	return 0;
}

/* 释放已经获取的磁盘块 */
void dbmap_free (uint blknum)
{
    struct disk_buf * buf;
	int bi, map;

	/* 获取该块对象对应位图所在的磁盘块 */
	buf = dbuf_alloc(BBLOCK(blknum, superblock));

	/* 清除磁盘块在位图中的已分配记录 */
	bi = blknum % (BSIZE * 8);
	map = 1 << (bi % 8);
	if((buf->data[bi / 8] & map) == 0)
    {
        kError(eSVC_fs, E_STATUS);
    }
	buf->data[bi / 8] &= ~map;

	/* 将位图所在的块写回磁盘 */
    dbuf_flush(buf);
    dbuf_free(buf);
}

#endif

/**************** 对磁盘块的操作 *****************/
#if 1
/* 将指定的磁盘块内容清空为零 */
void dblk_zero (uint blknum)
{
    struct disk_buf *buf = NULL;

    /* 将该磁盘块内容清空 */
    buf = dbuf_alloc(blknum);
    if (buf != NULL)
	{
        kmemset(buf->data, 0, BSIZE);
	}
	dbuf_flush(buf);
	dbuf_free(buf);
}

#endif
