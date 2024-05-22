/*
 *  管理并操作文件系统的目录项
 */
#include "defs.h"
#include "file.h"




/* 计算目录项添加到哈希表中的值 */
unsigned int ditem_hash(
    struct FileSystemDev *fsdev, char *path)
{
    return 0;
}

/* 申请一个目录项的内存空间，并做简单的初始化 */
struct DirectoryItem *ditem_alloc (char *name)
{
    return NULL;
}

/* 释放已经申请的目录项的内存空间 */
int ditem_free (struct DirectoryItem *dir)
{
    return 0;
}

/* 从哈希数组中获取指定的目录项 */
struct DirectoryItem *ditem_get (
        struct FileSystemDev *fsdev, char *path, uint flag)
{
    return NULL;
}

/* 释放已经获取的目录项 */
void ditem_put (struct DirectoryItem *ditem)
{

}

/* 将目录项添加到模块的哈希数组中 */
void ditem_add (struct DirectoryItem *ditem)
{

}

/* 获取目录项的绝对路径 */
char *ditem_path (struct DirectoryItem *ditem)
{
    return NULL;
}

/* 获取目录项绝对路径加目录项名的字符串 */
char *ditem_pathname (struct DirectoryItem *ditem)
{
    return NULL;
}

/* 初始化虚拟文件系统的目录项管理模块 */
void ditem_init (void)
{

}
