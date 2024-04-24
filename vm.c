
#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "elf.h"
#include "riscv.h"
#include "defs.h"


pagetable_t     kernel_pgtab;
extern char     etext[];
extern char     trampoline[];

/*
 *  解析指定页表中，虚拟地址 vAddr 所对应的物理地址
 *
 *  pagetable：要解析的一级页表首地址
 *  vAddr：要解析的虚拟地址
 *  alloc：是否要为空的页表条目申请新的内存空间
 */
static pte_t *_mmu (pagetable_t pagetable, uint64 vAddr, bool alloc)
{
    pte_t *pte = NULL;

    if (vAddr >= MAXVA)
        _error();

    for (int level = 2; level > 0; level--)
    {
        pte = &pagetable[PX(level, vAddr)];

        /* Whether pte is available */
        if (*pte & PTE_V)
        {
            pagetable = (pagetable_t)PTE2PA(*pte);
        }
        else
        {
            if (!alloc || (pagetable = (pde_t *)kalloc()) == 0)
                return 0;
            memset(pagetable, 0, PGSIZE);
            *pte = PA2PTE(pagetable) | PTE_V;
        }
    }
    return &pagetable[PX(0, vAddr)];
}

/* 为指定地址与大小的虚拟内存与物理内存建立映射关系 */
static int mappages (pagetable_t pagetable, uint64 vAddr, uint64 pAddr, uint64 size, int flag)
{
    uint64  start, end;
    pte_t   *pte = NULL;

    if (size == 0)
        _error();

    start = PGROUNDDOWN(vAddr);
    end   = PGROUNDDOWN(vAddr + size - 1);

    while(1)
    {
        pte = _mmu(pagetable, start, TRUE);
        if (pte == 0)
            return -1;
        if (*pte & PTE_V)
            _error();

        *pte = PA2PTE(pAddr) | flag | PTE_V;

        if (start == end)
            break;
        pAddr += PGSIZE;
        start += PGSIZE;
    }
    return 0;
}

/* 释放页表本身占用的物理地址 */
static void freepages (pagetable_t pagetable)
{
    int     i;
    pte_t   pte;
    uint64  pa;
    
    for (i=0; i<512; i++)
    {
        pte = pagetable[i];

        if ((pte & PTE_V) && ((pte & (PTE_R|PTE_W|PTE_X)) == 0))
        {
            pa = PTE2PA(pte);
            freepages((pagetable_t)pa);
            pagetable[i] = 0;
        }
        else if (pte & PTE_V)
        {
            return;
        }
    }
    kfree(pagetable);
}
/*******************************************************/
/* 获取虚拟地址对应的物理地址 */
uint64 kvm_phyaddr (pagetable_t pagetable, uint64 vAddr)
{
    pte_t   *pte;

    /* 虚拟地址的有效性检查 */
    if (vAddr > MAXVA)
        return 0;
    
    /* 获取第三级的页表条目 */
    pte = _mmu(pagetable, vAddr, FALSE);

    /* 页表条目的有效性检查 */
    if (! (*pte & PTE_V))
        return 0;
    if (! (*pte & PTE_U))
        return 0;
    
    /* 将页表条目内存储的物理地址返回 */
    return PTE2PA(*pte);
}

/* 将指定大小的物理地址与虚拟地址建立映射关系 */
void kvm_map (pagetable_t pagetable, uint64 vAddr, uint64 pAddr, uint64 sz, int flag)
{
    if (mappages(pagetable, vAddr, pAddr, sz, flag) < 0)
        _error();
}

/* 释放范围的页表条目所映射的物理内存页 */
void uvm_unmap (pagetable_t pagetable, uint64 va, uint64 npages, bool free)
{
    uint64  start, end;
    pte_t   *pte;

    if ((va % PGSIZE) != 0)
        return;
    if (npages <= 0)
        return;
    
    end = va + (npages * PGSIZE);
    for (start = va; start < end; start += PGSIZE)
    {
        pte = _mmu(pagetable, start, FALSE);

        if (pte == NULL)
            return;
        if (! (*pte & PTE_V))
            return;
        
        if (free == TRUE)
        {
            kfree((void*)PTE2PA(*pte));
        }

        *pte = 0;
    }
}

/* 创建新的页表 */
pagetable_t uvm_create (void)
{
    pagetable_t pgtab = (pagetable_t)kalloc();

    if (pgtab == NULL)
        return NULL;
    memset(pgtab, 0, PGSIZE);

    return pgtab;
}

/* 为指定范围的虚拟地址映射可用的物理地址 */
uint64 uvm_alloc (pagetable_t pagetable, uint64 oldaddr, uint64 newaddr, int flag)
{
    void    *mem;
    uint64  addr;

    if ((newaddr % PGSIZE) != 0)
        return 0;
    oldaddr = PGROUNDUP(oldaddr);

    for (addr=oldaddr; addr < newaddr; addr += PGSIZE)
    {
        mem = kalloc();
        if (mem == NULL)
            goto error_map;
        memset(mem, 0, PGSIZE);
    
        /* 建立映射关系 */
        if (mappages(pagetable, oldaddr, (uint64)mem, 4096, PTE_R|PTE_U|flag) != 0)
        {
            /* 释放已申请的内存页 */
            kfree(mem);
            goto error_map;
        }
    }
    return newaddr;
error_map:
    /* 处理之前已映射成功的内存页 */
    uvm_unmap(pagetable, oldaddr, (addr - oldaddr)/PGSIZE, TRUE);
    return 0;
}

/* 释放指定范围内虚拟地址所映射的物理地址 */
uint64 uvm_free (pagetable_t pagetable, uint64 oldsz, uint64 newsz)
{
    int npages;

    if (newsz <= oldsz)
        return 0;    
    if (PGROUNDUP(newsz) > PGROUNDUP(oldsz))
    {
        npages = (PGROUNDUP(newsz) - PGROUNDUP(oldsz)) / PGSIZE;
        uvm_unmap(pagetable, oldsz, npages, TRUE);
    }

    return newsz;
}

/* 销毁已申请的页表以及其映射的所有物理内存页 */
void uvm_destroy (pagetable_t pagetable, uint64 sz)
{
    uvm_free(pagetable, 0, sz);
    freepages(pagetable);
}

/* 完成两个内存页中指定大小内存的拷贝 */
int uvm_copy (pagetable_t destPage, pagetable_t srcPage, uint64 sz, bool alloc)
{
    uint64  i;
    pte_t   *d_pte = NULL, *s_pte = NULL;
    uint64  d_pa, s_pa;
    uint    s_flags;

    for (i=0; i<sz; i+=PGSIZE)
    {
        /* 获取源页表中的页表条目信息 */
        s_pte = _mmu(srcPage, i, FALSE);
        if ((s_pte == 0) || ((*s_pte & PTE_V)==0))
            return -1;
        s_pa = PTE2PA(*s_pte);
        s_flags = PTE_FLAGS(*s_pte);

        /* 是否需要给目标页表申请新的物理内存页 */
        if (alloc)
        {
            /* 申请新的内存页给目标页表 */
            d_pa = (uint64)kalloc();
            if (d_pa == 0)
                goto error_cpy;

            /* 完成内存页的数据拷贝 */
            memmove ((void*)d_pa, (void*)s_pa, PGSIZE);
            /* 为目标页表建立新的映射关系 */
            if (mappages(destPage, i, d_pa, PGSIZE, s_flags) != 0)
            {
                kfree((void*)d_pa);
                goto error_cpy;
            }
        }
        else
        {
            /* 获取目标页表的页表条目信息 */
            d_pte = _mmu(destPage, i, FALSE);
            if ((d_pte == NULL) || ((*d_pte & PTE_V)==0))
                goto error_cpy;
            d_pa = PTE2PA(*d_pte);

            /* 完成内存页的数据拷贝 */
            memmove ((void*)d_pa, (void*)s_pa, PGSIZE);
            /* 为目标页表建立新的映射关系 */
            if (mappages(destPage, i, d_pa, PGSIZE, s_flags) != 0)
                goto error_cpy;
        }
    }
    return 0;
error_cpy:
    uvm_free(destPage, 0, i);
    return -1;
}

/* 从内核空间拷贝数据到用户空间 */
int copyout (pagetable_t pagetable, uint64 dstva, char *src, uint64 len)
{
    uint64 n, va_base, pa;

    while (len)
    {
        va_base = PGROUNDDOWN(dstva);

        pa = kvm_phyaddr(pagetable, va_base);
        if (pa <= 0)
            return -1;
        
        n = PGSIZE - (dstva - va_base);
        if (n > len)
            n = len;
        
        memmove ((void*)(pa + (dstva - va_base)), src, n);

        src += n;
        len -= n;
        dstva += va_base + PGSIZE;
    }
    return 0;
}

/* 从用户空间拷贝数据到内核空间 */
int copyin (pagetable_t pagetable, char *dst, uint64 srcva, uint64 len)
{
    uint64 n, va0, pa0;

    while (len > 0)
    {
        /* 对齐用户空间缓冲区的虚拟地址 */
        va0 = PGROUNDDOWN(srcva);

        /* 获取该虚拟地址所对应的物理地址 */
        pa0 = kvm_phyaddr(pagetable, va0);
        if (pa0 == 0)
            return -1;

        /* dstva - va0 = 表示虚拟地址在页基上的偏移数量
         *
         * n 表示需要拷贝的数据大小 (同时处理了满一页与不满一页的情况)
         */
        n = PGSIZE - (srcva - va0);
        if (n > len)
            n = len;

        /* 将用户空间数据拷贝到内核空间 */
        memmove(dst, (void *)(pa0 + (srcva - va0)), n);

        /* 确认数据是否拷贝完成 */
        len -= n;
        dst += n;
        srcva = va0 + PGSIZE;
    }
    return 0;
}

/* 初始化虚拟内存管理模块 */
void kvm_init (void)
{
    kernel_pgtab = kalloc();
    memset(kernel_pgtab, 0, PGSIZE);

    // uart registers
    mappages(kernel_pgtab, UART0, UART0, PGSIZE, PTE_R | PTE_W);

    // virtio mmio disk interface
    mappages(kernel_pgtab, VIRTIO0, VIRTIO0, PGSIZE, PTE_R | PTE_W);

    // PLIC
    mappages(kernel_pgtab, PLIC, PLIC, 0x400000, PTE_R | PTE_W);

    // map kernel text executable and read-only.
    mappages(kernel_pgtab, KERNBASE, KERNBASE, (uint64)etext-KERNBASE, PTE_R | PTE_X);

    // map kernel data and the physical RAM we'll make use of.
    mappages(kernel_pgtab, (uint64)etext, (uint64)etext, PHYSTOP-(uint64)etext, PTE_R | PTE_W);

    // map the trampoline for trap entry/exit to
    // the highest virtual address in the kernel.
    mappages(kernel_pgtab, TRAMPOLINE, (uint64)trampoline, PGSIZE, PTE_R | PTE_X);

    sfence_vma();
    w_satp(MAKE_SATP(kernel_pgtab));
    sfence_vma();
}
