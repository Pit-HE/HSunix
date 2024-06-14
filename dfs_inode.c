/*
 * 存放磁盘文件系统关于索引节点的操作
 * dnode 中既存储普通的数据，也存储目录项 (disk_dirent)
 */
#include "defs.h"
#include "dfs_priv.h"

extern struct disk_sb *superblock;
extern struct disk_inode *root_node;


/* 清空单个磁盘索引节点占用的所有磁盘块 */
static void dinode_clear_single (struct disk_inode *dnode)
{
	int i, j;
	uint *a = NULL;
	struct disk_buf *buf = NULL;

	if (dnode == NULL)
		return;

	/* 释放 inode 占有的所有磁盘块 */
	for(i = 0; i < NDIRECT; i++)
	{
		if(dnode->addrs[i])
		{
			dbmap_free(dnode->addrs[i]);
			dnode->addrs[i] = 0;
		}
	}

	/* 处理最后一个磁盘块的释放 */
	if(dnode->ex_addr)
	{
		/* 读出最后一个磁盘块的内容 */
		buf = dbuf_alloc(dnode->ex_addr);
		a = (uint *)buf->data;

		/* 处理在最后一个磁盘块中的扩展 */
		for(j = 0; j < NINDIRECT; j++)
		{
			if(a[j])
			{
				dbmap_free(a[j]);
			}
		}
		/* 释放 buf 的占用 */
		dbuf_free(buf);

		/* 也释放最后一个磁盘块的映射 */
		dbmap_free(dnode->ex_addr);
		dnode->ex_addr = 0;
	}
	dnode->size = 0;

	/* 将内存中的磁盘索引节点信息写回磁盘 */
	dinode_flush(dnode);
}


/* 解析文件的绝对路径，获取该路径对象的父节点与对象名
 *
 * path: 文件对象的绝对路径
 * name: 存放该文件对象的名字
 * 
 * 返回值：该文件对象的父节点
 */
struct disk_inode *dinode_parent (char *path, char *name)
{
	char *tmp_path = path;
	struct disk_dirent *dir = NULL;
    struct disk_inode *node = root_node;
	struct disk_inode *oldnode = NULL;
	struct disk_dirent *olddir = NULL;

	/* 处理获取文件系统挂载路径的情况 */
	while(*tmp_path == '/' && *tmp_path)
        tmp_path++;
    if (*tmp_path == '\0')
	{
		name[0] = '\0';
        return node;
	}

    /* 循环解析路径中的下一个元素 */
	while((path = disk_path_getfirst(path, name)) != 0)
	{
		/* 解析到的 inode 必须是文件夹 */
		if(node->type != DISK_DIR)
			return NULL;

		/* 路径是否解析完成 */
		if(*path == '\0')
			return node;

		olddir = dir;
		oldnode = node;

		/* 在目录项中查找指定名字的 inode */
		dir = ddir_get(node, name);
		if (dir == NULL)
			return NULL;
		node = dinode_alloc(dir->inum);
		if(node == NULL)
		{
			ddir_put(node, dir, FALSE);
			return NULL;
		}

		/* 释放之前占用的内存空间 */
		ddir_put(node, olddir, FALSE);
		dinode_free(oldnode);
	}

	/* 是否需要释放对 inode 的占用 */
	return node;
}

/* 解析文件的绝对路径，获取该文件对象的磁盘索引节点与名字
 *
 * path: 文件对象的绝对路径
 * name: 存放该文件对象的名字
 * 
 * 返回值：该文件对象的磁盘索引节点
 */
struct disk_inode *dinode_path (char *path, char *name)
{
	char *tmp_path = path;
	struct disk_dirent *dir = NULL;
    struct disk_inode *node = root_node;
	struct disk_inode *oldnode = NULL;
	struct disk_dirent *olddir = NULL;

	/* 处理获取文件系统挂载路径的情况 */
	while(*tmp_path == '/' && *tmp_path)
        tmp_path++;
    if (*tmp_path == '\0')
	{
		name[0] = '\0';
        return node;
	}

    /* 循环解析路径中的下一个元素 */
	while((path = disk_path_getfirst(path, name)) != 0)
	{
		/* 解析到的 inode 必须是文件夹 */
		if(node->type != DISK_DIR)
			return NULL;

		/* 路径是否解析完成 */
		if(*name == '\0')
			return node;

		olddir = dir;
		oldnode = node;

		/* 在目录项中查找指定名字的 inode */
		dir = ddir_get(node, name);
		if (dir == NULL)
			return NULL;
		node = dinode_alloc(dir->inum);
		if(node == NULL)
		{
			ddir_put(node, dir, FALSE);
			return NULL;
		}

		/* 释放之前占用的内存空间 */
		ddir_put(node, olddir, FALSE);
		dinode_free(oldnode);
	}

	/* 是否需要释放对 inode 的占用 */
	return node;
}

/* 获取一个空闲磁盘索引节点的编号 
 * ( 与 dinode_put 成对使用 )
 */
uint dinode_get (uint type)
{
	int inum;
	struct disk_buf *buf = NULL;
	struct disk_inode *node = NULL;

	/* 遍历超级块上的所有 disk_inode */
	for(inum = 1; inum < superblock->ninodes; inum++)
	{
		/* 遍历索引节点磁盘块上记录的所有 disk_inode 信息 */
		buf = dbuf_alloc(IBLOCK(inum, superblock));
		node = (struct disk_inode *)buf->data + (inum % 
			(BSIZE / sizeof(struct disk_inode)));

		/* 寻找未分配的磁盘 dnode */
		if(node->type == 0)
		{
			/* 清空 disk_inode 的数据 */
			kmemset(node, 0, sizeof(struct disk_inode));

			/* 设置当前磁盘 disk_inode 的类型 */
			node->type = type;
			node->nlink = inum;

			/* mark it allocated on the disk */
			dbuf_flush(buf);
			dbuf_free(buf);

			/* 获取与 disk_inode 对应的内存 inode */
			return inum;
		}
		dbuf_free(buf);
	}
	kprintf("ialloc: no inodes\n");
	return -1;
}

/* 清除磁盘节点的内容，将其变回空闲节点 
 * ( 与 dinode_get 成对使用 )
 */
void dinode_put (struct disk_inode *dnode)
{
	struct disk_buf *buf = NULL;
	struct disk_inode *node = NULL;

	if (dnode == NULL)
		return;

	/* 释放磁盘节点占用的所有磁盘块 */
	dinode_clear_single(dnode);

	/* 获取内存 inode 对应的磁盘 disk_inode 所在的磁盘块 */
	buf = dbuf_alloc(IBLOCK(dnode->nlink, superblock));
	node = (struct disk_inode *)buf->data + (dnode->nlink %
		(BSIZE / sizeof(struct disk_inode)));

	/* 清空该磁盘节点的内容 */
	kmemset(node, 0, sizeof(struct disk_inode));

	/* 将磁盘块写回到日志系统 */
	dbuf_flush(buf);
	dbuf_free(buf);
}

/* 获取存放 disk_inode 的内存空间，并读取对应的磁盘信息
 * ( 与 dinode_free 成对使用 )
 *
 * inum：索引节点在磁盘索引块中的偏移值
 */
struct disk_inode *dinode_alloc (uint inum)
{
	struct disk_buf *buf = NULL;
	struct disk_inode *node = NULL;
	struct disk_inode *temp = NULL;

	/* 获取存放磁盘节点的空间 */
    node = (struct disk_inode *)kalloc(sizeof(struct disk_inode));
	if (node == NULL)
		return NULL;

	/* 是否需要读取磁盘内记录的索引节点的信息 */
	if (inum == 0)
		return node;

	/* 获取磁盘节点所在的磁盘块 */
	buf = dbuf_alloc(IBLOCK(inum, superblock));
	if (buf == NULL)
		return NULL;
	
	/* 获取磁盘节点具体的位置 */
	temp = (struct disk_inode *)buf->data + inum % (BSIZE / sizeof(struct disk_inode));

	/* 将磁盘节点的信息拷贝到内存对象中 */
	kmemcpy(node, temp, sizeof(struct disk_inode));
	node->nlink = inum;

	dbuf_free(buf);

	return node;
}

/* 释放存放磁盘 disk_inode 信息的内存空间 
 * ( 与 dinode_alloc 成对使用 )
 */
void dinode_free (struct disk_inode *dnode)
{
	if (dnode == NULL)
		return;

	/* 禁止释放根节点 */
	if (dnode == root_node)
		return;

	/* 释放磁盘节点占用的内存空间 */
	kfree(dnode);
}

/* 将磁盘节点在内存中的信息，写回到磁盘中 */
void dinode_flush (struct disk_inode *dnode)
{
	struct disk_buf *buf = NULL;
	struct disk_inode *node = NULL;

	/* 获取内存 inode 对应的磁盘 disk_inode 所在的磁盘块 */
	buf = dbuf_alloc(IBLOCK(dnode->nlink, superblock));
	node = (struct disk_inode *)buf->data + (dnode->nlink %
		(BSIZE / sizeof(struct disk_inode)));

	/* 将内存 inode 信息写入磁盘 disk_inode */
	kmemmove(node, dnode, sizeof(struct disk_inode));

	/* 将磁盘块写回到日志系统 */
	dbuf_flush(buf);
}


/* 读取磁盘节点 dnode 所占有的磁盘块中的数据 */
int dinode_read (struct disk_inode *dnode, char *dst, uint off, uint n)
{
	uint idx, m, addr;
	struct disk_buf *buf = NULL;

	/* 读取的数据大小是合理的 */
	if((off > dnode->size) || ((off + n) < off))
		return 0;

	/* 规范读取的数据大小 */
	if((off + n) > dnode->size)
		n = dnode->size - off;

	for(idx = 0; idx < n; idx += m, off += m, dst += m)
	{
		/* 获取要读的磁盘块编号 */
		addr = bmap(dnode, off / BSIZE);

		/* 完成数据的读取 */
		if(addr == 0)
			break;

		/* 读取指定磁盘块映射的 buf */
        buf = dbuf_alloc(addr);

		/* 计算当前 inode 剩余的可读空间 */
        if ((n - idx) < (BSIZE - off % BSIZE))
            m = n - idx;
        else
            m = BSIZE - (off % BSIZE);

		/* 将磁盘块的数据拷贝到用户空间的缓冲区 */
        kmemmove(dst, buf->data + (off % BSIZE), m);
        dbuf_free(buf);
	}
	return idx;
}

/* 将数据写入磁盘节点 dnode 所占有的磁盘块中 */
int dinode_write(struct disk_inode *dnode, char *src, uint off, uint n)
{
    uint tot, m;
	struct disk_buf *buf;

	/* 读取的数据大小是合理的 */
	if(off > dnode->size || off + n < off)
		return - 1;

	/* 规范读取的数据大小 */
	if(off + n > (NDIRECT + NINDIRECT) * BSIZE)
		return - 1;

	for(tot = 0; tot < n; tot += m, off += m, src += m)
	{
		/* 获取要写的磁盘块编号 */
		uint addr = bmap(dnode, off / BSIZE);

		/* 完成数据的写入 */
		if(addr == 0)
			break;

		/* 获取磁盘块上的内容 */
		buf = dbuf_alloc(addr);

		/* 计算当前 inode 剩余的可写空间 */
        if ((n - tot) < (BSIZE - off % BSIZE))
            m = n - tot;
        else
            m = BSIZE - off % BSIZE;

		/* 将用户缓冲区的数据写入磁盘块对应的 buf 中 */
        kmemmove(buf->data + (off % BSIZE), src, m);

		/* 将 buf 写回磁盘块中 */
		dbuf_flush(buf);
        dbuf_free(buf);
	}

    /* 更新磁盘节点所记录的数据大小 */
	if(off > dnode->size)
		dnode->size = off;

    /* 将磁盘索引节点的信息写回磁盘中 */
    dinode_flush(dnode);

	return tot;
}

/* 获取当前磁盘的根目录节点 */
struct disk_inode* dinode_getroot (void)
{
	#define ROOT_INUM 1
    uint blknum;
    struct disk_buf *buf = NULL;
    struct disk_inode *node = NULL;
	

	/* 是否已经获取到磁盘文件系统的根目录 */
	if (root_node != NULL)
		return root_node;

	/* 分配内存空间，获取磁盘内根节点的信息 */
    root_node = dinode_alloc(0);
    if (root_node == NULL)
		return NULL;

    /* 获取根目录节点所在的磁盘块 */
    blknum = (ROOT_INUM / (BSIZE / sizeof(struct disk_inode))) + superblock->inodestart;
    buf = dbuf_alloc(blknum);

    /* 获取根节点所在磁盘块的位置 */
    node = (struct disk_inode *)buf->data + (ROOT_INUM % (BSIZE / sizeof(struct disk_inode)));

    /* 拷贝根目录节点的信息 */
    kmemcpy(root_node, node, sizeof(struct disk_inode));

    /* 释放该缓冲区 */
    dbuf_free(buf);

    return root_node;
}

/* 将目录项中的所有文件与目录项递归释放 */
int dinode_release (struct disk_inode *dnode)
{
	uint off;
	struct disk_dirent tmp_dir;
	struct disk_inode *tmp_node = NULL;

	if (dnode == NULL)
		return -1;

	/* 遍历目录项中的所有文件对象 */
	for(off = 0; off < dnode->size; off += sizeof(struct disk_dirent))
	{
		/* 获取磁盘节点中记录的目录项信息 */
		if (dinode_read(dnode, (char*)&tmp_dir, off, sizeof(struct disk_dirent)) 
				!= sizeof(struct disk_dirent))
			return -1;

		/* 获取该目录项所对应的磁盘节点 */
		tmp_node = dinode_alloc(tmp_dir.inum);
		if (tmp_node == NULL)
			return -1;

		/* 递归遍历子目录项中的所有文件对象 */
		if (tmp_node->type == DISK_DIR)
		{
			if (-1 == dinode_release(tmp_node))
				return -1;
		}

		/* 释放磁盘索引节点所占用的内存空间与磁盘资源 */
		dinode_clear_single(tmp_node);
		dinode_free(tmp_node);
	}
	return 0;
}

