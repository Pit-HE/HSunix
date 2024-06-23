
#ifndef __UCORE_DEFS_H__
#define __UCORE_DEFS_H__

#include "types.h"
#include "riscv.h"
#include "file.h"
#include "proc.h"
#include "fs.h"
#include "timer.h"
#include "kerror.h"
#include "device.h"
#include "ringbuff.h"
#include "tinyprintf.h"



/******************** uart ********************/
void uart_init      (void);
void uartputc_async (int);
void uartputc_sync  (int);
int  uartgetc_intr  (void);
void uart_intrrupt  (void);


/******************** console ********************/
void init_console       (void);
void console_isr        (int c);
int  console_wString    (char *src);
int  console_wCmd       (char *src, int len);
void console_wChar      (void *, char  src);
int  console_rCmd       (char *src, int len);
int  console_rChar      (void);


/******************** string ********************/
void *kmemset (void *s, int c, uint n);
void *kmemmove(void *dst, const void *src, uint n);
void *kmemcpy (void *dst, const void *src, uint n);
int   kstrlen (const char *st);
char *kstrcpy (char *dest, const char *src);
int   kstrcmp (const char *p1, const char *p2);
int   kstrncmp(const char *p, const char *q, uint n);
char *kstrdup (const char *s);
char *kstrcat (char *dest, const char *src);
char *kstrchr (const char *str, int chr);
char *kstrrchr(const char *str, int ch);


/******************** kallocPhyPage ********************/
void *kallocPhyPage (void);
void  kfreePhyPage  (void *pa);
void *kalloc        (int size);
void  kfree         (void *obj);
void  init_kmem     (void);


/******************** vm ********************/
void     init_kvm    (void);
int      kvm_setflag (pgtab_t *pagetable, uint64 vAddr, int flag);
int      kvm_clrflag (pgtab_t *pagetable, uint64 vAddr, int flag);
uint64   kvm_phyaddr (pgtab_t *pagetable, uint64 vAddr);
int      kvm_map     (pgtab_t *pagetable, uint64 vAddr, uint64 pAddr, uint64 sz, int flag);
void     uvm_unmap   (pgtab_t *pagetable, uint64 vAddr, uint64 npages, bool free);
uint64   uvm_alloc   (pgtab_t *pagetable, uint64 start_addr, uint64 end_addr, int flag);
uint64   uvm_free    (pgtab_t *pagetable, uint64 oldsz, uint64 newsz);
void     uvm_destroy (pgtab_t *pagetable);
int      uvm_copy    (pgtab_t *destPage, pgtab_t *srcPage, uint64 sz, bool alloc);
int      uvm_copyout (pgtab_t *pagetable, uint64 dstva, char *src, uint64 len);
int      uvm_copyin  (pgtab_t *pagetable, char *dst, uint64 srcva, uint64 len);
pgtab_t *uvm_create  (void);

/******************** trap ********************/
void init_trap      (void);
void trap_userfunc  (void);
void trap_userret   (void);
void kerneltrap     (void);


/******************** test ********************/
void selfInspection (void);


/******************** proc ********************/
#define getCpuID()      r_tp()
CpuCB  *getCpuCB        (void);
int     proc_applypid   (void);
void    proc_wakeup     (ProcCB *pcb);
int     proc_killstate  (ProcCB *pcb);
void    do_scheduler (void);
void    do_defuncter (void);
void    do_yield     (void);
void    do_suspend   (void *obj);
void    do_resume    (void *obj);
int     do_fork      (void);
int     do_wait      (int *code);
void    do_exit      (int state);
int     do_kill      (int pid);
int     do_sleep     (int ms);
ProcCB *do_kthread   (char *name, void(*thread)(void));
void    init_proc    (void);

/******************** pcb ********************/
ProcCB  *getProcCB  (void);
void     pcb_dump   (void);
ProcCB  *pcb_alloc  (void);
int      pcb_free   (ProcCB *pcb);
ProcCB  *pcb_lookup (int pid);
int      proc_free_pgtab  (pgtab_t *pgtab);
pgtab_t *proc_alloc_pgtab (ProcCB *pcb);



/******************** plic ********************/
void init_plic      (void);
void init_plichart  (void);
int  plic_claim     (void);
void plic_complete  (int irq);


/******************** syscall ********************/
void do_syscall(void);


/******************** switch ********************/
void switch_to (Context *old, Context *new);


/******************** Port **********************/
void kPortDisableInterrupt (void);
void kPortEnableInterrupt (void);
#define kDISABLE_INTERRUPT()    kPortDisableInterrupt()
#define kENABLE_INTERRUPT()     kPortEnableInterrupt()


/******************** error *********************/
void kError (eService SVC, eCode code);
void kErrPrintf (char *fmt, ...);


/******************** ringbuff ******************/
void kRingbuf_init      (ringbuf_t *rb, char *buf, int len);
void kRingbuf_clean     (ringbuf_t *rb);
int  kRingbuf_put       (ringbuf_t *rb, char *buf, int len);
int  kRingbuf_get       (ringbuf_t *rb, char *buf, int len);
int  kRingbuf_putChar   (ringbuf_t *rb, char  ch);
int  kRingbuf_getChar   (ringbuf_t *rb, char *ch);
int  kRingbuf_delChar   (ringbuf_t *rb);
int  kRingbuf_putState  (ringbuf_t *rb);
int  kRingbuf_getState  (ringbuf_t *rb);
int  kRingbuf_getLength (ringbuf_t *rb);


/******************** cli ***********************/
void init_cli (void);
void cli_main (void);


/******************** init **********************/
void init_main (void);
void idle_main (void);
void user_main (void);


/******************** timer *********************/
void     init_timer (void);
timer_t *timer_add  (ProcCB *pcb, int expires);
void     timer_del  (timer_t *timer);
void     timer_run  (void);


/******************** exec **********************/
int do_exec(ProcCB *obj, char *path, char *argv[]);


/******************** device ********************/
void init_dev (void);
struct Device *dev_alloc (const char *name);
void dev_free (struct Device *dev);
void dev_register (struct Device *dev);
void dev_unregister (struct Device *dev);
struct Device *dev_get (const char *name);
void dev_put (struct Device *dev);

#endif
