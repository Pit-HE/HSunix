
#include "defs.h"
#include "file.h"


struct Inode *inode_alloc (void)
{
    struct Inode *inode = NULL;

    inode = (struct Inode *)kalloc(sizeof(struct Inode));
    if (inode == NULL)
        return NULL;
    memset(inode, 0, sizeof(struct Inode));

    inode->ref = 1;

    return inode;
}
void inode_free (struct Inode *inode)
{
    if (inode == NULL)
        return;

    kfree(inode);
}

int inode_open (struct Inode *inode)
{
    return 0;
}
void inode_close (struct Inode *inode)
{

}
int inode_read (struct Inode *inode, void *buf, uint len)
{
    return 0;
}
int inode_write (struct Inode *inode, void *buf, uint len)
{
    return 0;
}





