/*
 * 存放磁盘文件系统关于索引节点的操作
 * 1、disk_dirent 是磁盘 dnode 中一种特殊的数据
 * 2、dnode 中既存储普通的数据，也存储目录项 (disk_dirent)
 */
#include "defs.h"
#include "dfs_priv.h"

static uint bmap(struct disk_inode *dnode, uint bn)
{
	uint addr, *a;
	struct Iobuf *buf;

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

	/* 扩展 inode 数组记录的最后一个磁盘块，用来建立更多的磁盘块映射索引 */
	if(bn < NINDIRECT)
	{
		/* Load indirect block, allocating if necessary. */
		/* 处理最后一个数组成员还未分配磁盘空间的情况 */
		if((addr = dnode->addrs[NDIRECT]) == 0)
		{
			/* 获取空闲磁盘块编号 */
			addr = dbmap_alloc();
			if(addr == 0)
				return 0;

			/* 记录分配到的空闲磁盘块编号 */
			dnode->addrs[NDIRECT] = addr;
		}

		/* 读取磁盘块的内容 */
        buf = iob_read(addr);
		a = (uint *)buf->data;

		/* 将磁盘块格式化为一个个索引项，
		 并获取第一个空闲的索引项 */
		if((addr = a[bn]) == 0)
		{
			/* 获取空闲磁盘块的编号 */
			addr = dbmap_alloc();
			if(addr)
			{
				/* 将索引项与磁盘块建立联系 */
				a[bn] = addr;
				/* 将修改后的索引信息写回磁盘 */
                iob_write(buf);
			}
		}
        iob_release(buf);
		return addr;
	}

	/* inode 可用空间溢出，不支持继续扩展 */
	kErrPrintf("bmap: out of range");
    return -1;
}

/* 读取磁盘 dnode 中的数据 */
int dnode_read (struct disk_inode *dnode, char *dst, uint off, uint n)
{
	uint tot, m;
	struct Iobuf * buf;

	/* 读取的数据大小是合理的 */
	if(off > dnode->size || off + n < off)
		return 0;

	/* 规范读取的数据大小 */
	if(off + n > dnode->size)
		n = dnode->size - off;

	for(tot = 0; tot < n; tot += m, off += m, dst += m)
	{
		/* 获取要读的磁盘块编号 */
		uint addr = bmap(dnode, off / BSIZE);

		/* 完成数据的读取 */
		if(addr == 0)
			break;

		/* 读取指定磁盘块映射的 buf */
        buf = iob_read(addr);

		/* 计算当前 inode 剩余的可读空间 */
        if ((n - tot) < (BSIZE - off % BSIZE))
            m = n - tot;
        else
            m = BSIZE - off % BSIZE;

		/* 将磁盘块的数据拷贝到用户空间的缓冲区 */
        kmemmove(dst, buf->data, m);
        iob_release(buf);
	}
	return tot;
}

/* 将数据写入磁盘 dnode */
int dnode_write(struct disk_inode *dnode, char *src, uint off, uint n)
{
    uint tot, m;
	struct Iobuf *buf;

	/* 读取的数据大小是合理的 */
	if(off > dnode->size || off + n < off)
		return - 1;

	/* 规范读取的数据大小 */
	if(off + n > MAXFILE * BSIZE)
		return - 1;

	for(tot = 0; tot < n; tot += m, off += m, src += m)
	{
		/* 获取要写的磁盘块编号 */
		uint addr = bmap(dnode, off / BSIZE);

		/* 完成数据的写入 */
		if(addr == 0)
			break;

		/* 获取磁盘块上的内容 */
		buf = iob_read(addr);

		/* 计算当前 inode 剩余的可写空间 */
        if ((n - tot) < (BSIZE - off % BSIZE))
            m = n - tot;
        else
            m = BSIZE - off % BSIZE;

		/* 将用户缓冲区的数据写入磁盘块对应的 buf 中 */
        kmemmove(buf->data + (off % BSIZE), src, m);

		/* 将 buf 写回磁盘块中 */
        iob_write(buf);
        iob_release(buf);
	}

	if(off > dnode->size)
		dnode->size = off;

    dinode_updata(dnode);

	return tot;
}

/* 读取磁盘中 dir 的数据 */
struct disk_inode *ddir_read(struct disk_inode * dnode, 
        char * name, uint * poff)
{
    uint off, inum;
	struct disk_dirent dir;

	/* 确认当前的 inode 是文件目录 */
	if(dnode->type != T_DIR)
		kErrPrintf("dirlookup not DIR");

	/* 遍历 inode 内的所有目录条目 */
	for(off = 0; off < dnode->size; off += sizeof(dir))
	{
		/* 读取 inode 中目录条目的内容 */
        if (dnode_read(dnode, (char*)&dir, off, sizeof(dir)) != sizeof(dir))
            kErrPrintf("dirlookup read");

		/* 寻找非空闲的目录项 */
		if(dir.inum == 0)
			continue;

		/* 比较是否为寻找的目录条目 */
		if(kstrcmp(name, dir.name) == 0)
		{
			/* 返回目录项在 inode 中的偏移地址 */
			if(poff)
				*poff = off;

			/* 获取目录条目所对应的 inode 编号 */
			inum = dir.inum;
            if (inum)
                inum = 0;

			/* 获取目录项的 inode */
			return dinode_alloc(dnode->type);
		}
	}
	return NULL;
}

/* 将数据写入磁盘 dir */
int ddir_write(struct disk_inode *dnode, char *name, uint inum)
{
    int off;
	struct disk_dirent dir;
	struct disk_inode *ip;

	/* 检查要创建的目录项是否已经存在 */
	if((ip = ddir_read(dnode, name, 0)) != 0)
	{
		return - 1;
	}

	/* 遍历目录 inode 中的所有目录项空间 */
	for(off = 0; off < dnode->size; off += sizeof(dir))
	{
		/* 读取具体的目录项内容 */
        if (dnode_read(dnode, (char *)&dir, off, sizeof(dir)) != sizeof(dir))
            kErrPrintf("dirlink read");

		/* 查找空闲的目录项 */
		if(dir.inum == 0)
			break;
	}

	/* 设置目录项的名字 */
    kstrcpy(dir.name, name);

	/* 设置目录项分配到的 inode 编号 */
	dir.inum = inum;

	/* 将修改的目录项信息，写回到目录 inode */
    if (dnode_write(dnode, (char *)&dir, off, sizeof(dir)) != sizeof(dir))
    {
        return -1;
    }

	return 0;
}

