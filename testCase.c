
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "file.h"
#include "list.h"
#include "fcntl.h"
#include "fs.h"

/* 测试虚拟内存管理模块是否正常 */
void tc_virtualmemory (void)
{
    char *buf;
    Pagetable_t *pgtab, *Spgatab;

    pgtab = uvm_create();
    buf = kallocPhyPage();

    extern void kvm_map (Pagetable_t *pagetable, uint64 vAddr, uint64 pAddr, uint64 sz, int flag);
    kvm_map(pgtab, (uint64)0, (uint64)buf, PGSIZE, PTE_R | PTE_W | PTE_U);

    kstrcpy(buf, "hello World!\n");

    Spgatab = uvm_create();

    uvm_copy(Spgatab, pgtab, 20, TRUE);

    extern uint64  kvm_phyaddr (Pagetable_t *pagetable, uint64 va);
    buf = (char *)kvm_phyaddr(Spgatab, 0);

    kprintf ("%s", buf);

    uvm_free(pgtab, 0, PGSIZE);
    uvm_free(Spgatab, 0, PGSIZE);

    uvm_destroy(pgtab);
    uvm_destroy(Spgatab);
}


/* 测试动态内存管理模块是否正常 */
void tc_kalloc (void)
{
    /* 初步测试 */
 #if 0
    int i;
    char *ptr[10];

    ptr[0] = (char*)kalloc(32);
    kfree(ptr[0]);

 /* 奇数个测试 */
 #elif 0
    int i;
    char *ptr[10];

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
    int i;
    char *ptr[10];

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
    int i;
    char *ptr[10];

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
    int i;
    char *ptr[10];

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
    int i;
    char *ptr[10];

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
    int i;
    char *ptr[10];

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


/* 测试循环缓冲区模块是否正常 */
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
    kRingbuf_putChar(&tcRB, 0xA5);
    kRingbuf_put(&tcRB, tc_wBuf, 126);
    kRingbuf_putChar(&tcRB, 0x5A);

    kRingbuf_getChar(&tcRB, tc_rBuf);
    kRingbuf_get(&tcRB, tc_rBuf, 126);
    kRingbuf_getChar(&tcRB, tc_rBuf);
 #endif
}


/* 测试软件定时器的模块功能 */
void tc_timer (void)
{
    // int i;
    timer_t *tmr[10] = {0};
    // ProcCB *pcb;

    // ProcCB *allocProcCB (void);
    // pcb = allocProcCB();

 #if 0
    for(i=0; i<10; i++)
    {
        tmr[i] = timer_add(pcb, (i+1)*10);
    }
    for(i=0; i<10; i++)
    {
        timer_del(tmr[i]);
    }
 #elif 1
    timer_del(tmr[0]);
 #elif 0
    timer_run();
 #elif 0
    for(i=0; i<10; i++)
    {
        tmr[i] = timer_add(pcb, (i+1)*10);
    }
    for(i=0; i<30; i++)
    {
        timer_run();
    }
    for(i=0; i<10; i++)
    {
        timer_del(tmr[i]);
    }
 #endif
}


/* 测试文件系统的设备操作
 * ( 注：该用例需要在进程中调用 )
 */
void tc_fsDevice (void)
{
    int fd;
    char rbuf[20];
    char wbuf[20];

    kmemset(wbuf, 0, 20);
    kstrcpy(wbuf, "Hello World !\r\n");

    /* 查看设备打开与读写流程是否正常 */
    fd = vfs_open("console", O_WRONLY);
    vfs_write (fd, wbuf, kstrlen(wbuf));
    vfs_read(fd, rbuf, 1);
    vfs_close(fd);

    /* 测试进程的标准输入 */
    vfs_read(0, rbuf, 1);

    /* 测试进程的标准输出 */
    vfs_write(1, wbuf, kstrlen(wbuf));
}


/* 测试 ramfs 文件系统的函数接口 */
#if 1
static void _tc_ramfsPathParser (char *path)
{
    char parent[256], file[128], *pStr = NULL;

    kprintf ("PATH: %s\r\n", path);

    pStr = path_getfirst(path, file);
    kprintf ("first pStr = %s \r\n", pStr);
    kprintf ("first file = %s\r\n", file);

    path_getlast (path, parent, file);
    kprintf("last path = %s\r\n", parent);
    kprintf("last file = %s\r\n", file);
}
void tc_ramfs_api (void)
{
 #if 1
    /* 测试 ramfs 中字符串处理的接口 */
    char path[256];

    kstrcpy(path, "/home/HSunix/kernel");
    _tc_ramfsPathParser(path);

    kstrcpy(path, "/usr");
    _tc_ramfsPathParser(path);

    kstrcpy(path, "/");
    _tc_ramfsPathParser(path);
 #elif 0

 #endif
}
#endif

/* 系统自检接口 */
void self_inspection (void)
{
    // tc_virtualmemory();
    // tc_kalloc();
    // tc_ringbuff();
    // tc_timer();
    tc_ramfs_api();
}
