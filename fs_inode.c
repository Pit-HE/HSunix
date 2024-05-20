
#include "defs.h"
#include "file.h"


struct Inode *inode_alloc (void)
{
    struct Inode *inode = NULL;

    inode = (struct Inode *)kalloc(sizeof(struct Inode));
    if (inode == NULL)
        return NULL;
    kmemset(inode, 0, sizeof(struct Inode));

    inode->magic = INODE_MAGIC;

    return inode;
}

void inode_free (struct Inode *inode)
{
    if (inode == NULL)
        return;
    if (inode->magic != INODE_MAGIC)
        return;

    kmemset(inode, 0, sizeof(struct Inode));
    kfree(inode);
}

int inode_init (struct Inode *inode, unsigned int flags,
        struct FileOperation *fops, enum InodeType type)
{
    if ((inode == NULL) || (fops == NULL))
        return -1;

    inode->flags = flags;
    inode->fops  = fops;
    inode->type  = type;

    return 0;
}

