
#include "memlayout.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "list.h"



void os_logo (void)
{
    kprintf(" _    _    _____                   _         \r\n");
    kprintf("| |  | |  / ____|                 (_)        \r\n");
    kprintf("| |__| | | (___    _   _   _ __    _  __  __ \r\n");
    kprintf("|  __  |  \\___ \\  | | | | |  _ \\  | | \\ \\/ / \r\n");
    kprintf("| |  | |  ____) | | |_| | | | | | | |  >  <  \r\n");
    kprintf("|_|  |_| |_____/   \\__ _| |_| |_| |_| /_/\\_\\ \r\n");
    kprintf("============================================ \r\n");
}


void main (void)
{
    init_kmem();
    init_dev();
    init_console();
    // init_kvm();
    init_trap();
    init_plic();
    init_plichart();
    init_cli();
    init_timer();
    init_proc();
    init_vfs();

    power_selfInspection();
    os_logo();

    idle_main();
}

