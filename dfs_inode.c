/*
 * 存放磁盘文件系统关于索引节点的操作
 * 1、disk_dirent 是磁盘 dnode 中一种特殊的数据
 * 2、dnode 中既存储普通的数据，也存储目录项 (disk_dirent)
 */
#include "defs.h"
#include "dfs_priv.h"

struct dinode *root_node = NULL;


/* 返回 inode 中第 bn 个数据块的磁盘块号 */
static uint bmap(struct disk_sb *sb, struct dinode *dnode, uint bn)
{
	uint addr, *tmp;
	struct Iobuf *buf;

	/* 处理 inode 磁盘数组空间足够的情况 */
	if(bn < NDIRECT)
	{
		/* 处理索引的数据块还未分配磁盘空间的情况 */
		if((addr = dnode->addrs[bn]) == 0)
		{
			/* 获取设备中的空闲磁盘块编号 */
			addr = dbmap_alloc(sb);
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
			addr = dbmap_alloc(sb);
			if(addr == 0)
				return 0;

			/* 记录分配到的空闲磁盘块编号 */
			dnode->ex_addr = addr;
		}

		/* 读取磁盘块的内容 */
        buf = iob_alloc(addr);
		tmp = (uint *)buf->data;

		/* 将磁盘块格式化为一个个索引项，
		 并获取第一个空闲的索引项 */
		if((addr = tmp[bn]) == 0)
		{
			/* 获取空闲磁盘块的编号 */
			addr = dbmap_alloc(sb);
			if(addr)
			{
				/* 将索引项与磁盘块建立联系 */
				tmp[bn] = addr;
				/* 将修改后的索引信息写回磁盘 */
                iob_flush(buf);
			}
		}
        iob_free(buf);
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

char *disk_path_getlast (char *path, char *name)
{
	return path_getfirst(path, name);
}

/************************************************************
 *      对外的函数接口
 ***********************************************************/
/* 读取磁盘节点 dnode 所占有的磁盘块中的数据 */
int dnode_read (struct disk_sb *sb, struct dinode *dnode, char *dst, uint off, uint n)
{
	uint idx, m, addr;
	struct Iobuf * buf;

	/* 读取的数据大小是合理的 */
	if((off > dnode->size) || ((off + n) < off))
		return 0;

	/* 规范读取的数据大小 */
	if((off + n) > dnode->size)
		n = dnode->size - off;

	for(idx = 0; idx < n; idx += m, off += m, dst += m)
	{
		/* 获取要读的磁盘块编号 */
		addr = bmap(sb, dnode, off / BSIZE);

		/* 完成数据的读取 */
		if(addr == 0)
			break;

		/* 读取指定磁盘块映射的 buf */
        buf = iob_alloc(addr);

		/* 计算当前 inode 剩余的可读空间 */
        if ((n - idx) < (BSIZE - off % BSIZE))
            m = n - idx;
        else
            m = BSIZE - (off % BSIZE);

		/* 将磁盘块的数据拷贝到用户空间的缓冲区 */
        kmemmove(dst, buf->data + (off % BSIZE), m);
        iob_free(buf);
	}
	return idx;
}

/* 将数据写入磁盘节点 dnode 所占有的磁盘块中 */
int dnode_write(struct disk_sb *sb, struct dinode *dnode, 
        char *src, uint off, uint n)
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
		uint addr = bmap(sb, dnode, off / BSIZE);

		/* 完成数据的写入 */
		if(addr == 0)
			break;

		/* 获取磁盘块上的内容 */
		buf = iob_alloc(addr);

		/* 计算当前 inode 剩余的可写空间 */
        if ((n - tot) < (BSIZE - off % BSIZE))
            m = n - tot;
        else
            m = BSIZE - off % BSIZE;

		/* 将用户缓冲区的数据写入磁盘块对应的 buf 中 */
        kmemmove(buf->data + (off % BSIZE), src, m);

		/* 将 buf 写回磁盘块中 */
		iob_flush(buf);
        iob_free(buf);
	}

    /* 更新磁盘节点所记录的数据大小 */
	if(off > dnode->size)
		dnode->size = off;

    /* 将磁盘索引节点的信息写回磁盘中 */
    dnode_flush(sb, dnode);

	return tot;
}

/* 解析文件的绝对路径，返回对应的 dnode 对象 */
struct dinode *dnode_find (struct disk_sb *sb, 
        struct dinode *root, char *path, char *name)
{
	char *tmp_path = path;
    struct dinode *temp = root;

	/* 处理获取文件系统挂载路径的情况 */
	while(*tmp_path == '/' && *tmp_path)
        tmp_path++;
    if (*tmp_path == '\0')
	{
		name[0] = '\0';
        return root;
	}

    /* 循环解析路径中的下一个元素 */
	while((path = disk_path_getfirst(path, name)) != 0)
	{
		/* 解析到的 inode 必须是文件夹 */
		if(temp->type != T_DIR)
			return NULL;

		/* 路径是否解析完成 */
		if(*path == '\0')
			return temp;

		/* 在目录项中查找指定名字的 inode */
		if((temp = ddir_read(sb, temp, name, 0)) == 0)
			return NULL;
	}

	/* 是否需要释放对 inode 的占用 */
	return temp;
}

/* 释放之前申请的 dnode 对象 */
int dnode_release (struct disk_sb *sb, struct dinode *dnode)
{
    dnode_free(sb, dnode);
    return 0;
}

/* 获取当前磁盘的根目录节点 */
struct dinode* dnode_getroot (struct disk_sb *sb)
{
    uint blknum;
    struct Iobuf *buf = NULL;
    struct dinode *node = NULL;

	/* 是否已经获取到磁盘文件系统的根目录 */
	if (root_node != NULL)
		return root_node;

	/* 分配内存空间，获取磁盘内根节点的信息 */
    root_node = dnode_alloc(sb, 0);
    if (root_node == NULL)
		return NULL;

    /* 获取根目录节点所在的磁盘块 */
    blknum = ROOTINO / (BSIZE / sizeof(struct dinode)) + sb->inodestart;
    buf = iob_alloc(blknum);

    /* 获取根节点所在磁盘块的位置 */
    node = (struct dinode *)buf->data + ROOTINO % (BSIZE / sizeof(struct dinode));

    /* 拷贝根目录节点的信息 */
    kmemcpy(root_node, node, sizeof(struct dinode));

    /* 释放该缓冲区 */
    iob_free(buf);

    return root_node;
}

/* 读取磁盘中 dir 的数据 */
struct dinode *ddir_read(struct disk_sb *sb, struct dinode *dnode, 
    char *name, uint *poff)
{
    uint off;
	struct disk_dirent dir;

	/* 确认当前的 dinode 是文件目录 */
	if(dnode->type != T_DIR)
		kErrPrintf("dirlookup not DIR");

	/* 遍历 dinode 内的所有目录条目 */
	for(off = 0; off < dnode->size; off += sizeof(dir))
	{
		/* 读取 dinode 中目录条目的内容 */
        if (dnode_read(sb, dnode, (char*)&dir, off, sizeof(dir)) != sizeof(dir))
            kErrPrintf("dirlookup read");

		/* 寻找非空闲的目录项 */
		if(dir.inum == 0)
			continue;

		/* 比较是否为寻找的目录条目 */
		if(kstrcmp(name, dir.name) == 0)
		{
			/* 返回目录项在 inode 中的偏移地址 */
			if (poff)
                *poff = off;

			/* 获取目录项的 inode */
			return dnode_alloc(sb, dir.inum);
		}
	}
	return NULL;
}

/* 在目录节点中创建新的目录项 */
int ddir_write(struct disk_sb *sb, struct dinode *dnode, char *name, uint inum)
{
    int off;
	struct disk_dirent dir;
	struct dinode *ip;

	/* 检查要创建的目录项是否已经存在 */
	if((ip = ddir_read(sb, dnode, name, 0)) != 0)
	{
		return - 1;
	}

	/* 遍历目录 inode 中的所有目录项空间 */
	for(off = 0; off < dnode->size; off += sizeof(dir))
	{
		/* 读取具体的目录项内容 */
        if (dnode_read(sb, dnode, (char *)&dir, off, sizeof(dir)) != sizeof(dir))
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
    if (dnode_write(sb, dnode, (char *)&dir, off, sizeof(dir)) != sizeof(dir))
    {
        return -1;
    }

	return 0;
}

struct disk_dirent *ddir_get (struct disk_sb *sb, struct dinode *dnode, char *name)
{
	uint off;
	struct disk_dirent *dir;

	/* 确认当前的 dinode 是文件目录 */
	if(dnode->type != T_DIR)
		kErrPrintf("dirlookup not DIR");
	
	dir = (struct disk_dirent *)kalloc(sizeof(struct disk_dirent));
	if (dir == NULL)
		return NULL;

	/* 遍历 dinode 内的所有目录条目 */
	for(off = 0; off < dnode->size; off += sizeof(dir))
	{
		/* 读取 dinode 中目录条目的内容 */
        if (dnode_read(sb, dnode, (char*)dir, off, sizeof(dir)) != sizeof(dir))
            kErrPrintf("dirlookup read");

		/* 寻找非空闲的目录项 */
		if(dir->inum == 0)
			continue;

		/* 比较是否为寻找的目录条目 */
		if(kstrcmp(name, dir->name) == 0)
		{
			/* 获取目录项的 inode */
			return dir;
		}
	}
	return NULL;
}

void ddir_put (struct disk_sb *sb, struct dinode *dnode, struct disk_dirent *dir)
{
	uint off;
	struct dinode *node;

	if ((sb == NULL) || (dnode == NULL) || (dir == NULL))
		return;

	node = ddir_read(sb, dnode, dir->name, &off);
	if (node == 0)
		return;

	dnode_write(sb, dnode, (char*)dir, off, sizeof(struct disk_dirent));

	kfree(dir);
}

void ddir_rename (struct disk_sb *sb, struct dinode *dnode, struct disk_dirent *dir, char *name)
{
	uint off;
	struct dinode *node;

	if ((sb == NULL) || (dnode == NULL) || (dir == NULL))
		return;

	node = ddir_read(sb, dnode, dir->name, &off);
	if (node == 0)
		return;

	kstrcpy(dir->name, name);
	
	dnode_write(sb, dnode, (char *)dir, off, sizeof(struct disk_dirent));
}

