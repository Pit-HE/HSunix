/* 
 * 虚拟内存管理模块 (Virtual memory)
 */
#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "elf.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"
#include "kstring.h"
#include "pcb.h"


pgtab_t        *kernel_pgtab;
extern char     etext[];
extern char     trampoline[];

/*
 *  解析指定页表中，虚拟地址 vAddr 所对应的物理地址
 *
 *  pagetable：要解析的一级页表首地址
 *  vAddr：要解析的虚拟地址
 *  alloc：是否要为空的页表条目申请新的内存空间
 */
static pte_t *_mmu (pgtab_t *pgtab, uint64 vAddr, bool alloc)
{
    pte_t *pte = NULL;

    if (vAddr >= MAXVA)
        ErrPrint("_mmu: Invalid virtual address !\r\n");

    for (int level = 2; level > 0; level--)
    {
        pte = &pgtab[PX(level, vAddr)];

        /* Whether pte is available */
        if (*pte & PTE_V)
        {
            pgtab = (pgtab_t *)PTE2PA(*pte);
        }
        else
        {
            if (!alloc || (pgtab = (pde_t *)alloc_page()) == 0)
                return 0;
            kmemset(pgtab, 0, PGSIZE);
            *pte = PA2PTE(pgtab) | PTE_V;
        }
    }
    return &pgtab[PX(0, vAddr)];
}

/* 为指定地址与大小的虚拟内存与物理内存建立映射关系 */
static int mappages (pgtab_t *pgtab, uint64 vAddr, uint64 pAddr, uint64 size, int flag)
{
    uint64  start, end;
    pte_t   *pte = NULL;

    if (size == 0)
        ErrPrint("mappages: Invalid memory size !\r\n");

    start = PGROUNDDOWN(vAddr);
    end   = PGROUNDDOWN(vAddr + size - 1);

    while(1)
    {
        pte = _mmu(pgtab, start, TRUE);
        if (pte == 0)
            return -1;
        if (*pte & PTE_V)
            ErrPrint("mappages: Invalid PTE object !\r\n");

        *pte = PA2PTE(pAddr) | flag | PTE_V;

        if (start == end)
            break;
        pAddr += PGSIZE;
        start += PGSIZE;
    }
    return 0;
}

/* 释放页表本身占用的物理地址 */
static void freepages (pgtab_t *pgtab)
{
    int     i;
    pte_t   pte;
    uint64  pa;

    for (i=0; i<512; i++)
    {
        pte = pgtab[i];

        if ((pte & PTE_V) && ((pte & (PTE_R|PTE_W|PTE_X)) == 0))
        {
            pa = PTE2PA(pte);
            freepages((pgtab_t *)pa);
            pgtab[i] = 0;
        }
        else if (pte & PTE_V)
        {
            return;
        }
    }
    free_page(pgtab);
}


/*******************************************************/
/* 设置虚拟内存页中指定的权限标志 */
int kvm_setflag (pgtab_t *pgtab, uint64 vAddr, int flag)
{
    pte_t   *pte = NULL;

    pte = _mmu(pgtab, vAddr, FALSE);
    if (pte == NULL)
        return -1;
    
    *pte |= flag;
    return 0;
}

/* 清除虚拟内存页中指定的权限标志 */
int kvm_clrflag (pgtab_t *pgtab, uint64 vAddr, int flag)
{
    pte_t   *pte = NULL;

    pte = _mmu(pgtab, vAddr, FALSE);
    if (pte == NULL)
        return -1;
    
    *pte &= ~flag;
    return 0;
}

/* 获取虚拟地址所在的页表首地址 */
uint64 kvm_pageaddr (pgtab_t *pgtab, uint64 vAddr)
{
    pte_t *pte;

    /* 虚拟地址的有效性检查 */
    if (vAddr > MAXVA)
        return 0;

    /* 获取第三级的页表条目 */
    pte = _mmu(pgtab, vAddr, FALSE);

    /* 页表条目的有效性检查 */
    if (! (*pte & PTE_V))
        return 0;
    if (! (*pte & PTE_U))
        return 0;

    /* 将页表条目内存储的物理地址返回 */
    return PTE2PA(*pte);
}

/* 获取虚拟地址对应的物理地址 */
uint64 kvm_phyaddr (pgtab_t *pgtab, uint64 vAddr)
{
    uint64 pa;

    pa = kvm_pageaddr(pgtab, vAddr);
    pa |= vAddr & 0xFFF;

    /* 将页表条目内存储的物理地址返回 */
    return pa;
}

/* 将指定大小的物理地址与虚拟地址建立映射关系 */
int kvm_map (pgtab_t *pgtab, uint64 vAddr, uint64 pAddr, uint64 sz, int flag)
{
    return mappages(pgtab, vAddr, pAddr, sz, flag);
}

/* 释放指定地址范围内页表条目所映射的物理内存页 */
void uvm_unmap (pgtab_t *pgtab, uint64 vAddr, uint64 npages, bool free)
{
    uint64  start, end;
    pte_t   *pte;

    if ((vAddr % PGSIZE) != 0)
        return;
    if (npages <= 0)
        return;

    end = vAddr + (npages * PGSIZE);
    for (start = vAddr; start < end; start += PGSIZE)
    {
        pte = _mmu(pgtab, start, FALSE);

        if (pte == NULL)
            return;
        if (! (*pte & PTE_V))
            return;

        if (free == TRUE)
            free_page((void*)PTE2PA(*pte));

        *pte = 0;
    }
}

/* 创建新的页表 (与 uvm_destroy 成对使用) */
pgtab_t *uvm_create (void)
{
    pgtab_t *pgtab = (pgtab_t *)alloc_page();

    if (pgtab == NULL)
        return NULL;
    kmemset(pgtab, 0, PGSIZE);

    return pgtab;
}

/* 销毁已申请的页表所占用的内存页 (与 uvm_create 成对使用) */
void uvm_destroy (pgtab_t *pgtab, uint64 size)
{
    /* 释放每一个终端'页表项'所占用的物理内存页 */
    if (size)
        uvm_unmap(pgtab, 0, PGROUNDUP(size)/PGSIZE, TRUE);

    /* 释放页表本身所占用的内存页 */
    freepages(pgtab);
}

/* 为指定范围的虚拟地址映射可用的物理地址 */
uint64 uvm_alloc (pgtab_t *pgtab, uint64 start_addr, uint64 end_addr, int flag)
{
    char *mem = NULL;
    uint64  addr;

    if (start_addr > end_addr)
        return 0;
    start_addr = PGROUNDUP(start_addr);

    for (addr=start_addr; addr < end_addr; addr += PGSIZE)
    {
        mem = (char *)alloc_page();
        if (mem == NULL)
            goto error_map;
        kmemset(mem, 0, PGSIZE);

        /* 建立映射关系 */
        if (mappages(pgtab, addr, (uint64)mem, PGSIZE, flag) != 0)
        {
            /* 释放已申请的内存页 */
            free_page(mem);
            goto error_map;
        }
    }
    return end_addr;
 error_map:
    /* 处理之前已映射成功的内存页 */
    uvm_unmap(pgtab, start_addr, (addr - start_addr)/PGSIZE, TRUE);
    return 0;
}

/* 释放指定范围内虚拟地址所映射的物理地址 */
uint64 uvm_free (pgtab_t *pgtab, uint64 oldsz, uint64 newsz)
{
    int npages;

    if (newsz <= oldsz)
        return 0;
    if (PGROUNDUP(newsz) > PGROUNDUP(oldsz))
    {
        npages = (PGROUNDUP(newsz) - PGROUNDUP(oldsz)) / PGSIZE;
        uvm_unmap(pgtab, oldsz, npages, TRUE);
    }

    return newsz;
}


/* 完成两个内存页中指定大小内存的拷贝 */
int uvm_copy (pgtab_t *destPage, pgtab_t *srcPage, uint64 sz, bool alloc)
{
    uint64 i;
    uint s_flags;
    uint64 d_pa, s_pa;
    pte_t *d_pte = NULL;
    pte_t *s_pte = NULL;


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
            d_pa = (uint64)alloc_page();
            if (d_pa == 0)
                goto error_cpy;

            /* 完成内存页的数据拷贝 */
            kmemmove ((void*)d_pa, (void*)s_pa, PGSIZE);
            /* 为目标页表建立新的映射关系 */
            if (mappages(destPage, i, d_pa, PGSIZE, s_flags) != 0)
            {
                free_page((void*)d_pa);
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
            kmemmove ((void*)d_pa, (void*)s_pa, PGSIZE);

            /* 为目标页表建立新的映射关系 */
            if (mappages(destPage, i, d_pa, PGSIZE, s_flags) != 0)
                return -1;
        }
    }
    return 0;
 error_cpy:
    uvm_free(destPage, 0, i);
    return -1;
}

/* 从内核空间拷贝数据到用户空间 */
int uvm_copyout (pgtab_t *pgtab, uint64 dst_va, char *src, uint64 len)
{
    uint64 n, va_base, pa;

    while (len)
    {
        va_base = PGROUNDDOWN(dst_va);

        pa = kvm_pageaddr(pgtab, va_base);
        if (pa <= 0)
            return -1;

        n = PGSIZE - (dst_va - va_base);
        if (n > len)
            n = len;

        kmemmove ((void*)(pa + (dst_va - va_base)), src, n);

        src += n;
        len -= n;
        dst_va += va_base + PGSIZE;
    }
    return 0;
}

/* 从用户空间拷贝数据到内核空间 */
int uvm_copyin (pgtab_t *pgtab, char *dst, uint64 src_va, uint64 len)
{
    int ret = 0;
    uint64 n, va0, pa0;

    while (len > 0)
    {
        /* 对齐用户空间缓冲区的虚拟地址 */
        va0 = PGROUNDDOWN(src_va);

        /* 获取该虚拟地址所对应的物理地址 */
        pa0 = kvm_pageaddr(pgtab, va0);
        if (pa0 == 0)
            return -1;

        /* dst - va0 = 表示虚拟地址在页基上的偏移数量
         *
         * n 表示需要拷贝的数据大小 (同时处理了满一页与不满一页的情况)
         */
        n = PGSIZE - (src_va - va0);
        if (n > len)
            n = len;

        /* 将用户空间数据拷贝到内核空间 */
        kmemmove(dst, (void *)(pa0 + (src_va - va0)), n);

        /* 确认数据是否拷贝完成 */
        len -= n;
        dst += n;
        ret += n;
        src_va = va0 + PGSIZE;
    }
    return ret;
}

/* 从内核拷贝数据到用户空间 */
int copy_to_user (uint64 dst_va, char *src, uint64 len)
{
    return uvm_copyout(getProcCB()->pgtab, dst_va, src, len);
}

/* 从用户空间拷贝数据到内核 */
int copy_from_user (char *dst, uint64 src_va, uint64 len)
{
    return uvm_copyin(getProcCB()->pgtab, dst, src_va, len);
}



/* 初始化虚拟内存管理模块 */
void init_kvm (void)
{
    kernel_pgtab = uvm_create();

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

    uvm_alloc(kernel_pgtab, TRAPFRAME, TRAPFRAME+PGSIZE, PTE_R|PTE_W);

	kprintf ("init_kvm complete !\r\n");

    // Open vritual protect
    sfence_vma();
    w_satp(MAKE_SATP(kernel_pgtab));
    sfence_vma();
	kprintf ("w_satp start !\r\n");
}
