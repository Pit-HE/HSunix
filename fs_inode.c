
#include "defs.h"
#include "file.h"


struct Inode *inode_alloc (void)
{
    struct Inode *inode = NULL;

    inode = (struct Inode *)kalloc(sizeof(struct Inode));
    if (inode == NULL)
        return NULL;
    kmemset(inode, 0, sizeof(struct Inode));

    return inode;
}
void inode_free (struct Inode *inode)
{
    if (inode == NULL)
        return;

    kmemset(inode, 0, sizeof(struct Inode));
    kfree(inode);
}

/* 打开时 inode 引用计数递增 */
int inode_open (struct Inode *inode)
{
    if (inode == NULL)
        return -1;

    if (inode->i_ops->open != NULL)
        inode->i_ops->open(inode);
    
    return 0;
}
/* 关闭时 inode 引用计数递减 */
int inode_close (struct Inode *inode)
{
    if (inode == NULL)
        return -1;

    if (inode->i_ops->close != NULL)
        inode->i_ops->close(inode);
    
    return 0;
}
int inode_read (struct Inode *inode, void *buf, uint len)
{
    if (inode == NULL)
        return -1;

    if (inode->i_ops->read != NULL)
        inode->i_ops->read(inode, buf, len);

    return 0;
}
int inode_write (struct Inode *inode, void *buf, uint len)
{
    if (inode == NULL)
        return -1;

    if (inode->i_ops->write != NULL)
        inode->i_ops->write(inode, buf, len);

    return 0;
}
int  inode_flush (struct Inode *inode)
{
    if (inode == NULL)
        return -1;

    if (inode->i_ops->flush != NULL)
        inode->i_ops->flush(inode);

    return 0;
}




