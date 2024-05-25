
#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

extern char end[];
#ifndef ALIGN
#define ALIGN(addr, size) (((addr) + (size)-1) & (~((size)-1)))
#endif

typedef struct kernelMemoryNode
{
    uint64                  blkNum;
    struct kernelMemoryNode *next;
}kmNode;
/* 指向以页大小为单位的内存链表
    1、phyPageFreeHeader.blkNUM 表示当前页的数量
    2、phyPageFreeHeader.next 表示页链表的第一个块
 */

static kmNode phyPageFreeHeader;
/* 指向单向循环的空闲小内存块链表
    1、smallMemFreeHeader.blkNum 表示当前链表内的块数量
    2、smallMemFreeHeader.next 表示链表的第一个节点
    3、链表中每个节点的blkNum (node.blkNum) 表示当前节点块可用的内存大小
 */
static kmNode smallMemFreeHeader;
#define SL_HEADCODE 0x5AA5



/* 处理有效区间物理内存的申请功能 */
void *kallocPhyPage (void)
{
    kmNode *p;

    if (phyPageFreeHeader.next == NULL)
        return NULL;

    /* 从链表取出节点 */
    p = phyPageFreeHeader.next;
    phyPageFreeHeader.next = p->next;
    phyPageFreeHeader.blkNum -= 1;

    kmemset((void*)p, 5, PGSIZE);
    return p;
}

/* 处理有效区间物理内存的释放功能 */
void kfreePhyPage (void *pa)
{
    kmNode *p;

    /* 地址是否对齐 */
    if (((uint64)pa % PGSIZE) != 0)
        return;
    /* 是否为可用内存 */
    if ((PHYSTOP < (uint64)pa) || ((char*)pa < end))
        return;

    /* 格式化物理内存页 */
    kmemset (pa, 1, PGSIZE);
    p = (kmNode*)pa;

    /* 添加到空闲链表 */
    p->next = phyPageFreeHeader.next;
    phyPageFreeHeader.next = p;
    phyPageFreeHeader.blkNum += 1;
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
        kfreePhyPage(pa);
}


void kfree (void *obj)
{
    kmNode *objHear = (kmNode *)obj;
    kmNode *curHear = NULL;
    char *ptrHear = NULL;

    kDISABLE_INTERRUPT();
    if (obj == NULL)
        goto exit_kfree;
    objHear--;
    if ((objHear->next != (kmNode *)SL_HEADCODE) ||
            (objHear->blkNum == 0U))
        goto exit_kfree;
    kmemset(obj, 5, objHear->blkNum);

    /* 当链表中只有一个内存块时 */
    curHear = smallMemFreeHeader.next;
    if (curHear->next == &smallMemFreeHeader)
    {
        if (objHear < curHear)
        {
            ptrHear = (char *)objHear;
            ptrHear += objHear->blkNum;
            if (ptrHear == (char*)curHear)
            {
                objHear->blkNum += curHear->blkNum;
                objHear->next = curHear->next;
            }
            else
            {
                objHear->next = curHear;
                smallMemFreeHeader.blkNum += 1;
            }
            smallMemFreeHeader.next = objHear;
        }
        else
        {
            ptrHear = (char *)curHear;
            ptrHear += curHear->blkNum;
            if (ptrHear == (char*)objHear)
            {
                curHear->blkNum += objHear->blkNum;
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

    /* 遍历整个小内存块的链表,
       确认将 obj 放在 curHear 与 curHear->next 之间 */
    while(curHear->next != &smallMemFreeHeader)
    {
        if (objHear < curHear->next)
            break;
        curHear = curHear->next;
    }
    smallMemFreeHeader.blkNum += 1;

    ptrHear = (char *)objHear;
    ptrHear += objHear->blkNum;
    if (ptrHear == (char*)curHear->next)
    {
        objHear->blkNum += curHear->next->blkNum;
        objHear->next = curHear->next->next;
        smallMemFreeHeader.blkNum -= 1;
    }
    else
    {
        objHear->next = curHear->next;
    }

    ptrHear = (char *)curHear;
    ptrHear += curHear->blkNum;
    if (ptrHear == (char *)objHear)
    {
        curHear->blkNum += objHear->blkNum;
        curHear->next = objHear->next;
        smallMemFreeHeader.blkNum -= 1;
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
    int  objsize;
    char *ptr = NULL;
    void *retAddr = NULL;
    kmNode *curHear, *oldHear;

    if (size >= PGSIZE)
        goto exit_kalloc;
    if (smallMemFreeHeader.next == NULL)
        goto exit_kalloc;

    kDISABLE_INTERRUPT();
    objsize = size + sizeof(kmNode);
    for (curHear = &smallMemFreeHeader; ;)
    {
        if (curHear->next->blkNum >= objsize)
        {
            oldHear = curHear->next;
            if (oldHear->blkNum == objsize)
            {
                curHear->next = oldHear->next;
                smallMemFreeHeader.blkNum -= 1;
            }
            else
            {
                ptr = (char *)oldHear;
                curHear->next = (kmNode *)(ptr + objsize);
                curHear->next->blkNum = oldHear->blkNum - objsize;
                curHear->next->next = oldHear->next;
            }
            oldHear->blkNum = objsize;
            oldHear->next = (kmNode *)SL_HEADCODE;
            retAddr = (void*)++oldHear;
            kmemset(retAddr, 0, size);
            break;
        }

        if (curHear->next != &smallMemFreeHeader)
        {
            curHear = curHear->next;
        }
        else
        {
            kmNode *obj = (kmNode *)kallocPhyPage();
            if (obj == NULL)
                kError(eSVC_VirtualMem, E_INVAL);
            /* 设置为当前空闲内存块的大小 */
            obj->blkNum = PGSIZE;
            obj->next = (kmNode *)SL_HEADCODE;
            kfree ((char *)++obj);

            curHear = &smallMemFreeHeader;
        }
    }
    kENABLE_INTERRUPT();
exit_kalloc:
    return retAddr;
}

static void smallMemFormat (void)
{
    kmNode* curHear;

    curHear = (kmNode*)kallocPhyPage();
    if (curHear != NULL)
    {
        curHear->blkNum = PGSIZE;
        curHear->next = (kmNode *)SL_HEADCODE;

        kfree ((char *)++curHear);
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


