/*
 * 当前模块用于解析 elf 文件，
 * 并将其加载到进程的虚拟内存页中
 */
#include "fcntl.h"
#include "defs.h"
#include "fs.h"
#include "elf.h"


/* 加载 elf 文件中的段到页表虚拟地址中的指定位置 */
int load_elf_segment (Pagetable_t *pagetable, uint64 vAddr, 
        int fd, uint fd_off, uint segsize)
{
    uint i, len;
    uint64 pAddr;

    for (i=0; i < segsize; i += PGSIZE)
    {
        /* 获取页表虚拟地址所对应的物理地址 */
        pAddr = kvm_phyaddr(pagetable, vAddr + i);
        if (pAddr == 0)
            return -1;

        /* 获取要写入的长度 */
        len = (segsize < PGSIZE) ? segsize:PGSIZE;

        /* 设置要读取的段在文件中的偏移地址 */
        vfs_lseek(fd, fd_off + i, SEEK_SET);

        /* 读取文件的内容，写入页表的内存中 */
        if (-1 == vfs_read(fd, (void *)pAddr, len))
            return -1;
    }
    return 0;
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
    int fd, i, off, phdr_size;
    struct elf_ehdr elf;
    struct elf_phdr phdr;
    // Pagetable_t *pgtab = NULL;

    fd = vfs_open(path, O_RDWR, S_IRWXU);

    /* 获取 elf 的文件头信息 */
    if (-1 == vfs_read(fd, &elf, sizeof(elf)))
        goto _err_exec_open;

    /* 判断文件类型 */
    if (elf.e_magic != ELF_MAGIC)
        goto _err_exec_open;

    /* 创建新的虚拟内存页 */
    // pgtab = uvm_create();
    // if (pgtab == NULL)
    //     goto _err_exec_open;

    /* 遍历所有的段内容，
    并为虚拟内存页申请段内容所需的地址 */
    off = elf.e_phoff;
    phdr_size = sizeof(struct elf_phdr);
    for (i=0; i<elf.e_phnum; i++, off += phdr_size)
    {
        vfs_lseek(fd, off, SEEK_SET);
        if (-1 == vfs_read(fd, &phdr, phdr_size))
            goto _err_exec_uvm;
        

    }

    return 0;
_err_exec_uvm:
    // uvm_destroy(pgtab);
_err_exec_open:
    vfs_close(fd);
    return -1;
}







