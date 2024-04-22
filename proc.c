
#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "defs.h"
#include "proc.h"



struct cpu *mycpu (void)
{
    return 0;
}
struct proc *myproc (void)
{
    return 0;
}

uint32 yieldCnt = 0;
void yield(void)
{
    yieldCnt++;
}
int cpuid (void)
{
    return r_tp();
}
int kill(int pid)
{
    return 0;
}
void setkilled(struct proc *p)
{

}
int getkilled(struct proc *p)
{
    return 0;
}
int fork(void)
{
    return 0;
}
void exit(int status)
{

}