
#include "defs.h"
#include "file.h"
#include "fcntl.h"


struct Inode *inode_alloc (void)
{
    struct Inode *inode = NULL;

    inode = (struct Inode *)kalloc(sizeof(struct Inode));
    if (inode == NULL)
        return NULL;

    inode->magic = INODE_MAGIC;

    return inode;
}

void inode_free (struct Inode *inode)
{
    if (inode == NULL)
        return;
    if ((inode->magic != INODE_MAGIC) ||
        (inode->ref != 0))
        return;

    kfree(inode);
}

int inode_init (struct Inode *inode, unsigned int flag,
        struct FileOperation *fops, unsigned int mode)
{
    if ((inode == NULL) || (fops == NULL))
        return -1;
    if (inode->magic != INODE_MAGIC)
        return -1;

    inode->flags = flag;
    inode->fops  = fops;
    inode->mode  = mode;

    if (flag & O_DIRECTORY)
        inode->type = INODE_DIR;
    else
        inode->type = INODE_FILE;

    return 0;
}

