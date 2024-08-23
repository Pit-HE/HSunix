/*
 * 磁盘文件系统目录项的操作模块
 * disk_dirent 是磁盘 dnode 中一种特殊的数据
 */
#include "defs.h"
#include "dfs_priv.h"
#include "kstring.h"


/* 读取节点中指定名字的目录项信息 */
int ddir_read(struct disk_inode *dnode, struct disk_dirent *dir, char *name)
{
    uint off, dir_size;

	if ((dnode == NULL) || (dnode->type != DISK_DIR))
		return -1;
    if ((dir == NULL) || (name == NULL))
        return -1;

    dir_size = sizeof(struct disk_dirent);

	/* 遍历 disk_inode 内的所有目录项 */
	for(off = 0; off < dnode->size; off += dir_size)
	{
		/* 读取 disk_inode 中目录项的内容 */
        if (dinode_read(dnode, (char *)dir, off, dir_size) != dir_size)
            return -1;

		/* 寻找非空闲的目录项 */
		if(dir->inum == 0)
			continue;

		/* 比较是否为寻找的目录项 */
		if(kstrcmp(name, dir->name) == 0)
			return 0;
	}
	return -1;
}

/* 在指定的磁盘节点中创建新的目录项 */
int ddir_create (struct disk_inode *dnode, char *name, uint inum)
{
    int off, dir_size;
	struct disk_dirent dir;

	/* 检查要创建的目录项是否存在 */
	if(ddir_check(dnode, name) != -1)
		return - 1;
    dir_size = sizeof(struct disk_dirent);

	/* 遍历目录 inode 中的所有目录项空间 */
	for(off = 0; off < dnode->size; off += dir_size)
	{
		/* 读取具体的目录项内容 */
        if (dinode_read(dnode, (char *)&dir, off, dir_size) != dir_size)
            return -1;

		/* 查找空闲的目录项 */
		if(dir.inum == 0)
			break;
	}

	/* 写入目录项的信息 */
    kstrcpy(dir.name, name);
	dir.inum = inum;

	/* 将修改的目录项信息，写回到目录 inode */
    if (dinode_write(dnode, (char *)&dir, off, dir_size) != dir_size)
    {
        return -1;
    }
	return 0;
}

/* 遍历目录节点内的信息，获取现有的目录项 
 * (与 ddir_put 成对使用) 
 */
struct disk_dirent *ddir_get (struct disk_inode *dnode, char *name)
{
	uint off, dir_size;
	struct disk_dirent *dir;

	/* 确认当前的 disk_inode 是文件目录 */
	if ((dnode == NULL) || (name == NULL) ||
        (dnode->type != DISK_DIR))
		return NULL;

	dir = (struct disk_dirent *)kalloc(sizeof(struct disk_dirent));
	if (dir == NULL)
		return NULL;
    dir_size = sizeof(struct disk_dirent);

	/* 遍历 disk_inode 内的所有目录项 */
	for(off = 0; off < dnode->size; off += dir_size)
	{
		/* 读取 disk_inode 中目录项的内容 */
        if (dinode_read(dnode, (char*)dir, off, dir_size) != dir_size)
            return NULL;

		/* 寻找非空闲的目录项 */
		if(dir->inum == 0)
			continue;

		/* 比较是否为寻找的目录项 */
		if(kstrcmp(name, dir->name) == 0)
			return dir;
	}
	return NULL;
}

/* 释放已经获取到的目录项，并更新其在磁盘中的记录
 * ( 与 ddir_get 成对使用 )
 */
void ddir_put (struct disk_inode *dnode, struct disk_dirent *dir, bool clear)
{
	uint off;

	if ((dnode == NULL) || (dir == NULL))
		return;

	/* 获取目录项在其父节点中的偏移值 */
    off = ddir_check(dnode, dir->name);
	if (off == -1)
		return;

	/* 是否清除该目录项的内容 */
	if (clear == TRUE)
		kmemset(dir, 0, sizeof(struct disk_dirent));

	/* 将目录项的内容写回磁盘 */
	dinode_write(dnode, (char*)dir, off, sizeof(struct disk_dirent));

	/* 释放磁盘目录项占用的内存空间 */
	kfree(dir);
}

/* 将目录项的内容写回到磁盘中 */
void ddir_flush (struct disk_inode *dnode, struct disk_dirent *dir)
{
	uint off;

	if ((dnode == NULL) || (dir == NULL))
		return;

	/* 获取目录项在父节点中的偏移值 */
    off = ddir_check(dnode, dir->name);
	if (off == -1)
		return;

	/* 将目录项的内容写回到所属的磁盘块中 */
	dinode_write(dnode, (char*)dir, off, sizeof(struct disk_dirent));
}

/* 修改指定目录项的名字 */
void ddir_rename (struct disk_inode *dnode, struct disk_dirent *dir, char *name)
{
	uint off;

	if ((dnode == NULL) || (dir == NULL) || (name == NULL))
		return;

	/* 获取目录项在父节点中的偏移值 */
    off = ddir_check(dnode, dir->name);
	if (off == -1)
		return;

	kstrcpy(dir->name, name);
	
	dinode_write(dnode, (char *)dir, off, sizeof(struct disk_dirent));
}

/* 检查磁盘节点中是否存在指定名字的目录项 
 * 返回值：-1为失败，否则为目录项在磁盘节点存储空间的偏移值
 */
int ddir_check (struct disk_inode *dnode, char *name)
{
    uint off = 0, dir_size;
	struct disk_dirent dir;

	if ((dnode == NULL) || (name == NULL) ||
        (dnode->type != DISK_DIR))
		return -1;

    dir_size = sizeof(struct disk_dirent);

	/* 遍历 disk_inode 内的所有目录项 */
	for(off = 0; off < dnode->size; off += dir_size)
	{
		/* 读取 disk_inode 中目录项的内容 */
        if (dinode_read(dnode, (char*)&dir, off, dir_size) != dir_size)
            return -1;

		/* 寻找非空闲的目录项 */
		if(dir.inum == 0)
			continue;

		/* 是否为寻找的目录项，
           若为真则返回节点在节点存储空间中的偏移值 */
		if(kstrcmp(name, dir.name) == 0)
			return off;
	}
	return -1;
}
