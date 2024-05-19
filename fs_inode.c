
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






