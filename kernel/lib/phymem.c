/*
 * 内存动态管理模块，
 * 1、可以动态分配 4k 的页内存.
 * 2、可以动态分配指定大小的内存块.
 */
#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

extern char end[];
#ifndef ALIGN
#define ALIGN(addr, size) (((addr) + (size)-1) & (~((size)-1)))
#endif

/* 管理 4K 大小物理内存页的动态申请与释放 */
typedef struct kernelPageMemoryNode
{
    uint64 blkNum;
    struct kernelPageMemoryNode *next;
}kpm_node;
/* 
指向以页大小为单位的内存链表
    1、phyPageFreeHeader.blkNUM 表示当前页的数量
    2、phyPageFreeHeader.next 表示页链表的第一个块
 */
static kpm_node phyPageFreeHeader;


/* 管理小内存块的动态申请与释放 */
typedef struct kernelSmallMemoryNode
{
    uint16 magic;
    uint64 blkNum;
    struct kernelSmallMemoryNode *next;
}ksm_node;
/* 
指向单向循环的空闲小内存块链表
    1、smallMemFreeHeader.blkNum 表示当前链表内的块数量
    2、smallMemFreeHeader.next 表示链表的第一个节点
    3、链表中每个节点的blkNum (node.blkNum) 表示当前节点块可用的内存大小
 */
static ksm_node smallMemFreeHeader;
#define KSMN_MAGIC 0x5AA5



/* 处理有效区间物理内存的申请功能 */
void *alloc_page (void)
{
    kpm_node *p;

    if (phyPageFreeHeader.next == NULL)
        return NULL;

    kDISABLE_INTERRUPT();
    /* 从链表取出节点 */
    p = phyPageFreeHeader.next;
    phyPageFreeHeader.next = p->next;
    phyPageFreeHeader.blkNum -= 1;
    kENABLE_INTERRUPT();

    kmemset((void*)p, 5, PGSIZE);
    return p;
}

/* 处理有效区间物理内存的释放功能 */
void free_page (void *pa)
{
    kpm_node *p;

    /* 地址是否对齐 */
    if (((uint64)pa % PGSIZE) != 0)
        return;
    /* 是否为可用内存 */
    if ((PHYSTOP < (uint64)pa) || ((char*)pa < end))
        return;

    /* 格式化物理内存页 */
    kmemset (pa, 1, PGSIZE);
    p = (kpm_node*)pa;

    kDISABLE_INTERRUPT();
    /* 添加到空闲链表 */
    p->next = phyPageFreeHeader.next;
    phyPageFreeHeader.next = p;
    phyPageFreeHeader.blkNum += 1;
    kENABLE_INTERRUPT();
}

/* 将可用内存格式化为固定大小的页 */
static void phyPageFormat (void *pa_start, void *pa_end)
{
    char *pa;

    if ((pa_start == NULL) || (pa_end == NULL))
        return;
    /* 地址对齐 */
    pa = (char *)PGROUNDUP((uint64)pa_start);

    /* 将内存分页并链接到空闲链表中 */
    for (; pa + PGSIZE <= (char *)pa_end; pa += PGSIZE)
        free_page(pa);
}


void kfree (void *obj)
{
    ksm_node *objHear = (ksm_node *)obj;
    ksm_node *curHear = NULL;
    char *ptr = NULL;

    if (obj == NULL)
        return;
    objHear -= 1;
    if ((objHear->magic != KSMN_MAGIC) ||
        (objHear->blkNum == 0U))
        return;
    kmemset(obj, 5, objHear->blkNum - sizeof(ksm_node));

    kDISABLE_INTERRUPT();
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
        goto exit_kfree;
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

exit_kfree:
    kENABLE_INTERRUPT();
}

/* 只能用于申请小于 4096 大小的内存块 */
void *kalloc (int size)
{
    int  objsize = 0;
    char *ptr = NULL;
    ksm_node *currNode = NULL;
    ksm_node *tempNode = NULL;
    ksm_node *nextNode = NULL;

    if ((0 >= size) || (size >= PGSIZE))
        return NULL;
    if (smallMemFreeHeader.next == NULL)
        return NULL;

    kDISABLE_INTERRUPT();
    objsize = size + sizeof(ksm_node);
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
                (objsize + sizeof(ksm_node) + 1))
            {
                currNode->next = nextNode->next;
                smallMemFreeHeader.blkNum -= 1;
            }
            else
            {
                /* 分割该可用的空闲内存块 */
                ptr  = (char *)nextNode;
                tempNode = (ksm_node *)(ptr + objsize);

                /* 更新被分割后的内存块的头信息 */
                tempNode->magic  = KSMN_MAGIC;
                tempNode->blkNum = nextNode->blkNum - objsize;
                tempNode->next   = nextNode->next;

                currNode->next = tempNode;
                nextNode->blkNum = objsize;
            }
            /* 初始化寻找到的可用空闲内存块 */
            // nextNode->magic  = KSMN_MAGIC;
            nextNode->next   = NULL;
            nextNode        += 1;
            kmemset(nextNode, 0, size);
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
            ksm_node *obj = (ksm_node *)alloc_page();
            if (obj == NULL)
            {
                ErrPrint("kalloc: alloc_page fail !\r\n");
                return NULL;
            }
            /* 设置为当前空闲内存块的大小 */
            obj->magic  = KSMN_MAGIC;
            obj->blkNum = PGSIZE;
            obj->next   = NULL;
            obj        += 1;

            /* 将新的空闲内存加入管理链表 */ 
            kfree((char *)obj);

            /* 重新遍历当前空闲的链表 */
            currNode = &smallMemFreeHeader;
        }
    }
    kENABLE_INTERRUPT();
    return nextNode;
}

static void smallMemFormat (void)
{
    ksm_node* curHear;

    curHear = (ksm_node*)alloc_page();
    if (curHear != NULL)
    {
        curHear->magic  = KSMN_MAGIC;
        curHear->blkNum = PGSIZE;
        curHear->next   = NULL;
        curHear        += 1;

        kfree ((char *)curHear);
    }
}

/* 初始化整个物理内存管理模块 */
void init_kmem (void)
{
    phyPageFreeHeader.blkNum = 0;
    phyPageFreeHeader.next = NULL;
    phyPageFormat(end, (void*)PHYSTOP);

    smallMemFreeHeader.blkNum = 0U;
    smallMemFreeHeader.next = &smallMemFreeHeader;
    smallMemFormat();
}


