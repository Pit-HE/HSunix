
#include "memlayout.h"
#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "list.h"
#include "cli.h"



void os_logo (void)
{
    cli_clear();
    kprintf(" _    _    _____                   _         \r\n");
    kprintf("| |  | |  / ____|                 (_)        \r\n");
    kprintf("| |__| | | (___    _   _   _ __    _  __  __ \r\n");
    kprintf("|  __  |  \\___ \\  | | | | |  _ \\  | | \\ \\/ / \r\n");
    kprintf("| |  | |  ____) | | |_| | | | | | | |  >  <  \r\n");
    kprintf("|_|  |_| |_____/   \\__ _| |_| |_| |_| /_/\\_\\ \r\n");
    kprintf("\r\n");
    kprintf("HSunix running in RISC64-V architecture.\r\n");
    kprintf("Usr 'help' to list all command.\r\n");
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

    selfInspection();
    os_logo();

    idle_main();
}

