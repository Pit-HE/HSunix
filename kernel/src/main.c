
#include "memlayout.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "list.h"
#include "cli.h"



void main (void)
{
    init_kmem();
    init_dev();
    init_console();
    init_kvm();
    init_trap();
    init_plic();
    init_plichart();
    init_timer();
    init_proc();
    init_vfs();

    selfInspection();

    idle_main();
}

