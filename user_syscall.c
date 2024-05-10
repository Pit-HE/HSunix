
#include "syscall.h"


extern void syscall (int code, ...);


void syscall_fock (void)
{
    syscall(SYS_fork);
}
void syscall_exit (int code)
{
    syscall(SYS_exit, code); 
}
void syscall_sleep (int ms)
{
    syscall(SYS_sleep, ms);
}
