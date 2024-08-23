/* 
 * 记录所有与进程相关的系统调用
 */
#include "syspriv.h"
#include "syscall.h"
#include "defs.h"
#include "proc.h"
#include "pcb.h"
#include "msg.h"


uint64 sys_exit (void)
{
    int num;

    arg_int(0, &num);
    do_exit(num);
    return 0;
}
uint64 sys_fork (void)
{
    return do_fork();
}
uint64 sys_wait (void)
{
    uint64 code;

    arg_addr(0, &code);
    code = kvm_phyaddr(getProcCB()->pgtab, code);

    return do_wait((int *)code);
}
uint64 sys_yield (void)
{
    do_yield();
    return 0;
}
uint64 sys_kill (void)
{
    int pid;

    arg_int(0, &pid);
    return do_kill(pid);
}
uint64 sys_getpid (void)
{
    return getProcCB()->pid;
}
uint64 sys_putc (void)
{
    int ch;

    arg_int(0, &ch);
    console_wChar(NULL, ch);
    return 0;
}
uint64 sys_getc (void)
{
    return console_rChar();
}
uint64 sys_suspend (void)
{
    uint64 obj;

    arg_addr(0, &obj);
    do_suspend((void *)obj);
    return 0;
}
uint64 sys_resume (void)
{
    uint64 obj;

    arg_addr(0, &obj);
    do_resume((void *)obj);
    return 0;
}
uint64 sys_gettime (void)
{
    extern uint64 Systicks;
    return Systicks;
}
uint64 sys_sleep (void)
{
    int num;

    arg_int(0, &num);
    return do_sleep(num);
}
uint64 sys_brk (void)
{
    uint64 vaddr;
    struct ProcCB *pcb = getProcCB();

    /* 在进程用户空间占用的页表之上，扩展可用的动态内存 */
    vaddr = PGROUNDUP(pcb->memSize);
    uvm_alloc(pcb->pgtab, vaddr,
        vaddr + PGSIZE, PTE_R | PTE_W | PTE_U);

    /* 记录经常页表的用于动态内存而扩张的大小 */
    kDISABLE_INTERRUPT();
    pcb->memSize += PGSIZE;
    kENABLE_INTERRUPT();

    return vaddr;
}
uint64 sys_msgget(void)
{
    int key, flag;

    arg_int(0, &key);
    arg_int(1, &flag);

    return k_msgget(key, flag);
}
uint64 sys_msgsnd(void)
{
    uint64 addr;
    int id, size;

    arg_int (0, &id);
    arg_addr(1, &addr);
    arg_int (2, &size);

    addr = kvm_phyaddr(getProcCB()->pgtab, addr);

    return k_msgsnd(id, (void*)addr, size, 0);
}
uint64 sys_msgrcv(void)
{
    uint64 addr;
    int id, size, type;

    arg_int (0, &id);
    arg_addr(1, &addr);
    arg_int (2, &size);
    arg_int (3, &type);

    addr = kvm_phyaddr(getProcCB()->pgtab, addr);

    return k_msgrcv(id, (void*)addr, size, type, 0);
}
uint64 sys_msgctl(void)
{
    uint64 addr;
    int id, cmd;

    arg_int (0, &id);
    arg_int (1, &cmd);
    arg_addr(2, &addr);

    addr = kvm_phyaddr(getProcCB()->pgtab, addr);

    return k_msgctl(id, cmd, (void*)addr);
}
