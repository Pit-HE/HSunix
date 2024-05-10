
#ifndef __UCORE_DEFS_H__
#define __UCORE_DEFS_H__

#include "types.h"
#include "riscv.h"
#include "proc.h"
#include "timer.h"
#include "kerror.h"
#include "ringbuff.h"


/******************** uart ********************/
void uart_init      (void);
void uartputc_async (int);
void uartputc_sync  (int);
int  uartgetc_intr  (void);
void uart_intrrupt  (void);


/******************** console ********************/
void console_init       (void);
void console_isr        (int c);
int  console_wString    (char *src);
int  console_wCmd       (char *src, int len);
void console_wChar      (char  src);
int  console_rCmd       (char *src, int len);
int  console_rChar      (void);


/******************** string ********************/
void *memset (void *s, int c, uint n);
void *memmove(void *dst, const void *src, uint n);
void *memcpy (void *dst, const void *src, uint n);
int   strlen (const char *st);
char *strcpy (char *dest, const char *src);


/******************** kallocPhyPage ********************/
void *kallocPhyPage (void);
void  kfreePhyPage  (void *pa);
void *kalloc        (int size);
void  kfree         (void *obj);
void  kmem_init     (void);


/******************** vm ********************/
void    kvm_init    (void);
Pagetable_t *uvm_create (void);
void    uvm_unmap   (Pagetable_t *pagetable, uint64 va, uint64 npages, bool free);
uint64  uvm_alloc   (Pagetable_t *pagetable, uint64 oldaddr, uint64 newaddr, int flag);
uint64  uvm_free    (Pagetable_t *pagetable, uint64 oldsz, uint64 newsz);
void    uvm_destroy (Pagetable_t *pagetable);
int     uvm_copy    (Pagetable_t *destPage, Pagetable_t *srcPage, uint64 sz, bool alloc);
int     copyout     (Pagetable_t *pagetable, uint64 dstva, char *src, uint64 len);
int     copyin      (Pagetable_t *pagetable, char *dst, uint64 srcva, uint64 len);


/******************** trap ********************/
void trap_init      (void);
void trap_userfunc  (void);
void trap_userret   (void);
void kerneltrap     (void);


/******************** proc ********************/
#define getCpuID()  r_tp()
CpuCB_t  *getCpuCB  (void);
ProcCB_t *getProcCB (void);
void    wakeProcCB  (ProcCB_t *pcb);
void    dumpProcCB  (void);
void    scheduler   (void);
void    defuncter   (void);
void    proc_init   (void);
void    do_yield    (void);
void    do_suspend  (void *obj);
void    do_resume   (void *obj);
int     do_fork     (void);
int     do_wait     (int *code);
void    do_exit     (int state);
int     do_kill     (int pid);
int     do_sleep    (int ms);
int     KillState   (ProcCB_t *pcb);
void    wakeProcCB  (ProcCB_t *pcb);

/******************** plic ********************/
void plic_init      (void);
void plic_inithart  (void);
int  plic_claim     (void);
void plic_complete  (int irq);


/******************** syscall ********************/
void do_syscall(void);


/******************** kprintf ********************/
void kprintf (char*, ...);


/******************** switch ********************/
void switch_to (Context_t *old, Context_t *new);


/******************** Port **********************/
void kPortDisableInterrupt (void);
void kPortEnableInterrupt (void);
#define kDISABLE_INTERRUPT()    kPortDisableInterrupt()
#define kENABLE_INTERRUPT()     kPortEnableInterrupt()


/******************** error *********************/
void kError (eService SVC, eCode code);


/******************** ringbuff ******************/
void kRingbuf_init      (ringbuf_t *rb, char *buf, int len);
void kRingbuf_clean     (ringbuf_t *rb);
int  kRingbuf_put       (ringbuf_t *rb, char *buf, int len);
int  kRingbuf_get       (ringbuf_t *rb, char *buf, int len);
int  kRingbuf_putChar   (ringbuf_t *rb, char  ch);
int  kRingbuf_getChar   (ringbuf_t *rb, char *ch);
int  kRingbuf_putState  (ringbuf_t *rb);
int  kRingbuf_getState  (ringbuf_t *rb);

/******************** cli ***********************/
void cli_init (void);
void cli_main (void);


/******************** init **********************/
void init_main (void);
void idle_main (void);
void test_main (void);


/******************** timer *********************/
void     timer_init (void);
timer_t *timer_add  (ProcCB_t *pcb, int expires);
void     timer_del  (timer_t *timer);
void     timer_run  (void);

/******************** exec **********************/
int do_exec(char *path, char **argv);


#endif
