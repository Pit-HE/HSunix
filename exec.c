/*
 * 当前模块用于解析 elf 文件，
 * 并将其加载到进程的虚拟内存页中
 */
#include "fcntl.h"
#include "defs.h"
#include "fs.h"
#include "elf.h"
#include "cli.h"

/* 加载 elf 文件中的段到页表虚拟地址中的指定位置 */
int elf_load_segment (pgtab_t *pagetable, uint64 vAddr,  
    uint segsize, int fd, uint fd_off)
{
    uint i, len;
    uint64 pAddr;

    for (i=0; i < segsize; i += PGSIZE)
    {
        /* 获取页表虚拟地址所对应的物理地址 */
        pAddr = kvm_phyaddr(pagetable, vAddr + i);
        if (pAddr == 0)
            return -1;

        /* 计算要写入的长度 */
        len = ((segsize - i) < PGSIZE) ? segsize:PGSIZE;

        /* 设置要读取的段在文件中的偏移地址 */
        vfs_lseek(fd, fd_off + i, SEEK_SET);

        /* 读取文件的内容，写入页表的内存中 */
        if (-1 == vfs_read(fd, (void *)pAddr, len))
            return -1;
    }
    return 0;
}

/* 创建页表中与栈相关的虚拟地址
 *
 * pagetable：要设置的页表
 * vAddr：可以用于栈空间的虚拟地址
 * 
 * 返回值：返回当前页表中的栈顶地址
 */
uint64 elf_stack_create (pgtab_t *pagetable, uint64 vAddr)
{
    uint64 tmp;

    /* 设置进程的栈监测页 (大小为一页 4096) */
    tmp = PGROUNDUP(vAddr);
    uvm_alloc(pagetable, tmp, tmp + PGSIZE, 0);
    /* 限制访问该栈监测页，否则触发页故障 */
    kvm_clrflag(pagetable, tmp, PTE_U);

    /* 设置进程的栈空间 (大小为一页 4096) */
    tmp += PGSIZE;
    uvm_alloc(pagetable, tmp, tmp + PGSIZE, PTE_W);

    return (tmp += PGSIZE);
}

/* 将要传递的参数数组写到进程的栈空间内 
 *
 * pagetable：要设置的页表
 * top: 当前的页表内的栈顶地址
 * argv: 要写入栈内的数组内容
 * 
 * 返回值：当前写入栈内的数组成员个数
 */
int elf_para_create (pgtab_t *pagetable, uint64 *sptop, char *argv[])
{
    int argc;
    uint64 len, sp = *sptop;
    uint64 array[CLI_ARG_MAX];

    /* 将参数数组写入栈空间内，记录每一个数组成员存放的地址 */
    for (argc = 0; argv[argc]; argc++)
    {
        len = kstrlen(argv[argc]) + 1;
        sp -= len;
        /* 按照 riscv 的格式执行 16 位对齐 */
        sp -= sp % 16;
        array[argc] = sp;

        if (0 > uvm_copyout(pagetable, sp, argv[argc], len))
            return -1;
    }
    array[argc] = 0;
    
    /* 将记录参数数组存放地址的数据写入栈空间内 */
    len = (argc + 1) * sizeof(uint64);
    sp -= len;
    /* 按照 riscv 的格式执行 16 位对齐 */
    sp -= sp % 16;
    if (0 > uvm_copyout(pagetable, sp, (char *)array, len))
        return -1;

    *sptop = sp;
    return argc;
}

/* 解析 elf 文件的内容，并将其加载到
 * 
 * path: 记录 elf 内容的文件路径
 * argv：要传递给新进程的参数数组
 *
 * 返回值：argv 中记录的参数数量
 */
int do_exec(char *path, char *argv[])
{
    int fd, i;
    uint64 argc, off, sptop;
    struct elf_ehdr elf;
    struct elf_phdr phdr;
    ProcCB *pcb = getProcCB();
    pgtab_t *pgtab = NULL;

    fd = vfs_open(path, O_RDWR, S_IRWXU);

    /* 获取 elf 的文件头信息 */
    if (-1 == vfs_read(fd, &elf, sizeof(elf)))
        goto _err_exec_open;

    /* 判断文件类型 */
    if (elf.e_magic != ELF_MAGIC)
        goto _err_exec_open;

    /* 创建新的虚拟内存页 */
    pgtab = proc_alloc_pgtab(pcb);
    if (pgtab == NULL)
        goto _err_exec_open;

    /* 遍历所有的段内容，并为虚拟内存页申请段内容所需的地址 */
    off = elf.e_phoff;
    for (i=0; i<elf.e_phnum; i++, off += sizeof(struct elf_phdr))
    {
        /* 读取段头列表中的单个段头 */
        vfs_lseek(fd, off, SEEK_SET);
        if (-1 == vfs_read(fd, &phdr, sizeof(struct elf_phdr)))
            goto _err_exec_uvm;

        /* 该段是否可加载 */
        if (phdr.p_type != ELF_PROG_LOAD)
            continue;
        /* 段的地址必须是页对齐的 */
        if ((phdr.p_vaddr % PGSIZE) != 0)
            goto _err_exec_uvm;

        /* 在虚拟地址中为当前段映射相应的物理内存 */
        if (0 >= uvm_alloc(pgtab, phdr.p_vaddr, 
                phdr.p_vaddr + phdr.p_memsz, PTE_X | PTE_W))
            goto _err_exec_uvm;

        /* 将数据段写入页表所对应的虚拟内存中 */
        if (-1 == elf_load_segment(pgtab, phdr.p_vaddr,
                phdr.p_memsz, fd, off))
            goto _err_exec_uvm;
    }
    
    /* 设置页表中关于栈的内容，代码空间设置完后就设置栈 */
    sptop = elf_stack_create(pgtab, phdr.p_vaddr + phdr.p_memsz);

    /* 将要传递的参数数组写入栈空间 */
    argc = elf_para_create(pgtab, &sptop, argv);
    if (argc < 0)
        goto _err_exec_uvm; 

    /* 释放进程原先的虚拟内存页资源 */
    proc_free_pgtab(pcb->pageTab);

    /* 将虚拟页表的信息写入进程中 */
    pcb->pageTab = pgtab;
    pcb->memSize = phdr.p_memsz;
    pcb->stackSize = PGSIZE;
    pcb->trapFrame->epc = elf.e_entry;
    pcb->trapFrame->sp  = sptop;

    vfs_close(fd);
    return argc;

_err_exec_uvm:
    proc_free_pgtab(pgtab);

_err_exec_open:
    vfs_close(fd);
    return -1;
}
