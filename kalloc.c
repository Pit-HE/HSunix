
#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"

extern char end[];


struct kmNode
{
    uint64          blkNum;
    struct kmNode   *next;
};
static struct kmNode kmFreeHeader;


/* 处理有效区间物理内存的申请功能 */
void *kalloc (void)
{
    struct kmNode *p;

    if (kmFreeHeader.next == NULL)
        return NULL;

    /* 从链表取出节点 */
    p = kmFreeHeader.next;
    kmFreeHeader.next = p->next;
    kmFreeHeader.blkNum -= 1;

    memset((void*)p, 5, PGSIZE);
    return p;
}

/* 处理有效区间物理内存的释放功能 */
void kfree (void *pa)
{
    struct kmNode *p;

    /* 地址是否对齐 */
    if (((uint64)pa % PGSIZE) != 0)
        return;
    /* 是否为可用内存 */
    if ((PHYSTOP < (uint64)pa) || ((char*)pa < end))
        return;

    /* 格式化物理内存页 */
    memset (pa, 1, PGSIZE);
    p = (struct kmNode*)pa;
    
    /* 添加到空闲链表 */
    p->next = kmFreeHeader.next;
    kmFreeHeader.next = p;
    kmFreeHeader.blkNum += 1;
}

/* 将可用内存格式化为固定大小的页 */
static void kmFormat (void *pa_start, void *pa_end)
{
    char *pa;

    if ((pa_start == NULL) || (pa_end == NULL))
        return;
    /* 地址对齐 */
    pa = (char *)PGROUNDUP((uint64)pa_start);

    /* 将内存分页并链接到空闲链表中 */
    for (; pa + PGSIZE <= (char *)pa_end; pa += PGSIZE)
        kfree(pa);
}

/* 初始化整个物理内存管理模块 */
void kinit (void)
{
    kmFreeHeader.next = NULL;
    kmFreeHeader.blkNum = 0;

    kmFormat(end, (void*)PHYSTOP);
}




int alloc (int size)
{
    return 0;
}
int free (char *obj)
{
    return 0;
}
