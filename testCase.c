
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "list.h"



void tc_virtualmemory (void)
{
    char *buf;
    pagetable_t pgtab, Spgatab;

    pgtab = uvm_create();
    buf = kallocPhyPage();
    kvm_map(pgtab, (uint64)0, (uint64)buf, PGSIZE, PTE_R | PTE_W | PTE_U);

    strcpy(buf, "hello World!\n");

    Spgatab = uvm_create();

    uvm_copy(Spgatab, pgtab, 20, TRUE);
    buf = (char *)kvm_phyaddr(Spgatab, 0);

    kprintf ("%s", buf);

    uvm_destroy(pgtab, PGSIZE);
    uvm_destroy(Spgatab, PGSIZE);
}


void tc_kalloc (void)
{
    // int i;
    // char *ptr[10];

/* 初步测试 */
#if 0
    ptr[0] = (char*)kalloc(32);
    kfree(ptr[0]);

/* 奇数个测试 */
#elif 0
    for (i=0; i<7; i++)
    {
        ptr[i] = (char*)kalloc(32);
    }
    for (i=0; i<7; i++)
    {
        kfree(ptr[i]);
    }

/* 偶数个测试 */
#elif 0
    for (i=0; i<10; i++)
    {
        ptr[i] = (char*)kalloc(64);
    }
    for (i=0; i<10; i++)
    {
        kfree(ptr[i]);
    }

/* 大数据块测试 */
#elif 0
    for (i=0; i<10; i++)
    {
        ptr[i] = (char*)kalloc(2048);
    }
    for (i=0; i<10; i++)
    {
        kfree(ptr[i]);
    }

/* 临界值测试 */
#elif 0
    for (i=0; i<6; i++)
    {
        /* 一次分走一半物理内存页大小 */
        ptr[i] = (char*)kalloc(2048 - 16);
    }
    for (i=0; i<6; i++)
    {
        kfree(ptr[i]);
    }

/* 极限值测试 */
#elif 0
    for (i=0; i<10; i++)
    {
        /* 一次分走一个物理内存页大小 */
        ptr[i] = (char*)kalloc(4096 - 16);
    }
    for (i=0; i<10; i++)
    {
        kfree(ptr[i]);
    }

/* 非顺序操作 */
#elif 0
    for (i=0; i<10; i++)
    {
        ptr[i] = (char*)kalloc(32);
    }

    kfree(ptr[0]);
    kfree(ptr[3]);
    kfree(ptr[4]);
    kfree(ptr[7]);
    kfree(ptr[9]);

    ptr[0] = (char*)kalloc(64);
    ptr[3] = (char*)kalloc(32);
#endif
}


void tc_ringbuff (void)
{
    ringbuf_t tcRB;
    char tc_rbBuf[128], tc_wBuf[128], tc_rBuf[128];

    kRingbuf_init(&tcRB, tc_rbBuf, 128);

#if 0
    kRingbuf_put(&tcRB, tc_wBuf, 128);
    kRingbuf_get(&tcRB, tc_rBuf, 128);
#elif 1
    kRingbuf_put(&tcRB, tc_wBuf, 100);
    kRingbuf_get(&tcRB, tc_rBuf, 50);

    kRingbuf_put(&tcRB, tc_wBuf, 50);
    kRingbuf_get(&tcRB, tc_rBuf, 80);
#elif 0
    kRingbuf_putchar(&tcRB, 0xA5);
    kRingbuf_put(&tcRB, tc_wBuf, 126);
    kRingbuf_putchar(&tcRB, 0x5A);

    kRingbuf_getchar(&tcRB, tc_rBuf);
    kRingbuf_get(&tcRB, tc_rBuf, 126);
    kRingbuf_getchar(&tcRB, tc_rBuf);
#endif
}
