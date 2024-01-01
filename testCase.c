
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "list.h"



void tc_init (void)
{
    // kvm_init();
}


void tc_main (void)
{
    char *buf;
    pagetable_t pgtab, Spgatab;

    pgtab = uvm_create();
    buf = kalloc(); 
    kvm_map(pgtab, (uint64)0, (uint64)buf, 4096, PTE_R | PTE_W | PTE_U);

    strcpy(buf, "hello World!\n");

    Spgatab = uvm_create();
    // uvm_alloc(Spgatab, 0, 4096, PTE_R | PTE_W | PTE_U);
    // buf = (char *)kvm_phyaddr(Spgatab, 0);

    uvm_copy(Spgatab, pgtab, 20, TRUE);
    buf = (char *)kvm_phyaddr(Spgatab, 0);

    console_wCmd(buf, 15);
    
    uvm_destroy(pgtab, 4096);
    uvm_destroy(Spgatab, 4096);
}

 