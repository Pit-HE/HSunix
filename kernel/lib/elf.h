
#ifndef __ELF_H__
#define __ELF_H__


#include "types.h"


/* elf 文件头的魔幻数 */
#define ELF_MAGIC               0x464C457FU 
/* elf 文件的程序段类型：可加载 */
#define ELF_PROG_LOAD           1

/* elf 文件头：记录整个 elf 文件的信息 */
struct elf_ehdr
{
    /* 魔幻数 */
    uint e_magic;
    /*ELF的一些标识信息，固定值*/
    uchar e_ident[12];
    /*目标文件类型：1-可重定位文件，2-可执行文件，3-共享目标文件等*/
    ushort e_type;
    /*文件的目标体系结构类型：3-intel 80386*/
    ushort e_machine;
    /*目标文件版本：1-当前版本*/
    uint e_version;
    /*程序入口的虚拟地址，如果没有入口，可为0*/
    uint64 e_entry;
    /*程序头表(segment header table)的偏移量，没有可为0*/
    uint64 e_phoff;
    /*节区头表(section header table)的偏移量，没有可为0*/
    uint64 e_shoff;
    /*与文件相关的，特定于处理器的标志*/
    uint e_flags;
    /*ELF头部的大小，单位字节*/
    ushort e_ehsize;
    /*程序头表每个表项的大小，单位字节*/
    ushort e_phentsize;
    /*程序头表表项的个数*/
    ushort e_phnum;
    /*节区头表每个表项的大小，单位字节*/
    ushort e_shentsize;
    /*节区头表表项的数目*/
    ushort e_shnum;
    /*某些节区中包含固定大小的项目，如符号表。对于这类节区，此成员给出每个表项的长度字节数。*/
    ushort e_shstrndx;
};

/* 程序段头：记录每一个代码编号后的段信息 */
struct elf_phdr
{
    /*segment的类型：PT_LOAD= 1 可加载的段*/
    uint32  p_type;
    /*段标志*/
    uint32  p_flags;
    /*从文件头到该段第一个字节的偏移*/
    uint64  p_offset;
    /*该段第一个字节被放到内存中的虚拟地址*/
    uint64  p_vaddr;
    /*在linux中这个成员没有任何意义，值与p_vaddr相同*/
    uint64  p_paddr;
    /*该段在文件映像中所占的字节数*/
    uint64  p_filesz;
    /*该段在内存映像中占用的字节数*/
    uint64  p_memsz;
    /*p_vaddr是否对齐*/
    uint64  p_align;
};



#endif
