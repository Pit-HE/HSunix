
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "list.h"



void tc_main (void)
{
    char *buf;
    pagetable_t pgtab, Spgatab;

    pgtab = uvm_create();
    buf = kalloc();
    kvm_map(pgtab, (uint64)0, (uint64)buf, PGSIZE, PTE_R | PTE_W | PTE_U);

    strcpy(buf, "hello World!\n");

    Spgatab = uvm_create();

    uvm_copy(Spgatab, pgtab, 20, TRUE);
    buf = (char *)kvm_phyaddr(Spgatab, 0);

    kprintf ("%s", buf);

    uvm_destroy(pgtab, PGSIZE);
    uvm_destroy(Spgatab, PGSIZE);
}

