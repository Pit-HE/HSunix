
#include "libc.h"
#include "types.h"

/* 管理小内存块的动态申请与释放 */
typedef struct kernelSmallMemoryNode
{
    uint16 magic;
    uint64 blkNum;
    struct kernelSmallMemoryNode *next;
}usm_node;
/* 
指向单向循环的空闲小内存块链表
    1、smallMemFreeHeader.blkNum 表示当前链表内的块数量
    2、smallMemFreeHeader.next 表示链表的第一个节点
    3、链表中每个节点的blkNum (node.blkNum) 表示当前节点块可用的内存大小
 */
static usm_node smallMemFreeHeader;
#define USMN_MAGIC  0x5AA5
#define PGSIZE      4096

void free (void *obj)
{
    usm_node *objHear = (usm_node *)obj;
    usm_node *curHear = NULL;
    char *ptr = NULL;

    if (obj == NULL)
        return;
    objHear -= 1;
    if ((objHear->magic != USMN_MAGIC) ||
        (objHear->blkNum == 0U))
        return;
    memset(obj, 5, objHear->blkNum - sizeof(usm_node));

    curHear = smallMemFreeHeader.next;

    /* 处理空闲管理链中只有一个内存块的情况 */
    if (curHear->next == &smallMemFreeHeader)
    {
        if (objHear < curHear)
        {/* 将 obj 插入 curHear 之前 */
            ptr = (char *)objHear;
            ptr += objHear->blkNum;

            /* 判断两个内存块是否相临 */
            if (ptr == (char*)curHear)
            {
                /* 合并 obj 与 curHear 的内存空间 */
                objHear->blkNum += curHear->blkNum;
                objHear->next = curHear->next;

                curHear->magic  = 0;
                curHear->blkNum = 0;
                curHear->next   = NULL;
            }
            else
            {
                /* 将 obj 插入空闲链表 */
                objHear->next = curHear;
                smallMemFreeHeader.blkNum += 1;
            }
            smallMemFreeHeader.next = objHear;
        }
        else
        {/* 将 obj 插入 curHear 之后 */
            ptr = (char *)curHear;
            ptr += curHear->blkNum;

            /* 判断两个内存块是否地址相临 */
            if (ptr == (char*)objHear)
            {
                /* 合并 obj 与 curHear */
                curHear->blkNum += objHear->blkNum;

                objHear->magic  = 0;
                objHear->blkNum = 0;
                objHear->next   = 0;
            }
            else
            {
                objHear->next = curHear->next;
                curHear->next = objHear;
                smallMemFreeHeader.blkNum += 1;
            }
        }
        return;
    }

    /********************************************
    *   处理管理链表中有多个空闲内存块相链接的情况
    ********************************************/
    /* 遍历整个小内存块的链表, 确认 obj 插入 curHear 的位置 */
    while(curHear->next != &smallMemFreeHeader)
    {
        if (objHear < curHear->next)
            break;
        curHear = curHear->next;
    }
    smallMemFreeHeader.blkNum += 1;

    /* 确认将 obj 放在 curHear 节点之前*/
    ptr = (char *)objHear;
    ptr += objHear->blkNum;

    /* 判断两个内存块是否地址相临 */
    if (ptr == (char*)curHear->next)
    {
        objHear->blkNum += curHear->next->blkNum;
        objHear->next    = curHear->next->next;
        smallMemFreeHeader.blkNum -= 1;

        curHear->next->magic  = 0;
        curHear->next->blkNum = 0;
        curHear->next->next   = NULL;
    }
    else
    {
        objHear->next = curHear->next;
    }

    /* 确认将 obj 放在 curHear 节点之后*/ 
    ptr = (char *)curHear;
    ptr += curHear->blkNum;

    /* 判断两个内存块是否地址相临 */
    if (ptr == (char *)objHear)
    {
        curHear->blkNum += objHear->blkNum;
        curHear->next = objHear->next;
        smallMemFreeHeader.blkNum -= 1;

        objHear->magic  = 0;
        objHear->blkNum = 0;
        objHear->next   = NULL;
    }
    else
    {
        curHear->next = objHear;
    }
}

/* 只能用于申请小于 4096 大小的内存块 */
void *malloc (int size)
{
    int  objsize = 0;
    char *ptr = NULL;
    usm_node *currNode = NULL;
    usm_node *tempNode = NULL;
    usm_node *nextNode = NULL;

    if ((0 >= size) || (size >= PGSIZE))
        return NULL;
    if (smallMemFreeHeader.next == NULL)
        return NULL;

    objsize = size + sizeof(usm_node);
    for (currNode = &smallMemFreeHeader; ;)
    {
        /* 是否有足够大的空闲内存 */
        if (currNode->next->blkNum >= objsize)
        {
            /* 获取要操作的空闲内存块 */
            nextNode = currNode->next;

            /* 判断该空闲内存是否有足够的可分配空间，
             * 若该内存剩余空间过小，则整块分配
             */
            if (nextNode->blkNum < 
                (objsize + sizeof(usm_node) + 1))
            {
                currNode->next = nextNode->next;
                smallMemFreeHeader.blkNum -= 1;
            }
            else
            {
                /* 分割该可用的空闲内存块 */
                ptr  = (char *)nextNode;
                tempNode = (usm_node *)(ptr + objsize);

                /* 更新被分割后的内存块的头信息 */
                tempNode->magic  = USMN_MAGIC;
                tempNode->blkNum = nextNode->blkNum - objsize;
                tempNode->next   = nextNode->next;

                currNode->next = tempNode;
                nextNode->blkNum = objsize;
            }
            /* 初始化寻找到的可用空闲内存块 */
            nextNode->magic  = USMN_MAGIC;
            nextNode->next   = NULL;
            nextNode        += 1;
            memset(nextNode, 0, size);
            break;
        }

        /* 获取下一个空闲内存块 */
        if (currNode->next != &smallMemFreeHeader)
        {
            currNode = currNode->next;
        }
        else
        {
            /* 没有可用的空闲内存块时，申请一页新的物理内存 */
            usm_node *obj = (usm_node *)brk();
            if (obj == NULL)
            {
                printf("kalloc: brk fail !\r\n");
                return NULL;
            }
            /* 设置为当前空闲内存块的大小 */
            obj->magic  = USMN_MAGIC;
            obj->blkNum = PGSIZE;
            obj->next   = NULL;
            obj        += 1;

            /* 将新的空闲内存加入管理链表 */ 
            free((char *)obj);

            /* 重新遍历当前空闲的链表 */
            currNode = &smallMemFreeHeader;
        }
    }
    return nextNode;
}

void init_memory (void)
{
    usm_node* curHear = NULL;

    smallMemFreeHeader.blkNum = 0U;
    smallMemFreeHeader.next = &smallMemFreeHeader;

    curHear = (usm_node*)brk();
    if (curHear != NULL)
    {
        curHear->magic  = USMN_MAGIC;
        curHear->blkNum = PGSIZE;
        curHear->next   = NULL;
        curHear        += 1;

        free ((char *)curHear);
    }
}
