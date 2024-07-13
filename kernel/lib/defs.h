
#ifndef __UCORE_DEFS_H__
#define __UCORE_DEFS_H__

#include "types.h"
#include "riscv.h"
#include "file.h"
#include "proc.h"
#include "fs.h"
#include "timer.h"
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
void init_console    (void);
void console_isr     (int c);
int  console_wString (char *src);
int  console_wCmd    (char *src, int len);
void console_wChar   (void *, char  src);
int  console_rCmd    (char *src, int len);
int  console_rChar   (void);


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


/******************** alloc_page ********************/
void *alloc_page (void);
void  free_page  (void *pa);
void *kalloc        (int size);
void  kfree         (void *obj);
void  init_kmem     (void);


/******************** vm ********************/
void     init_kvm    (void);
pgtab_t *uvm_create  (void);
int      kvm_setflag (pgtab_t *pgtab, uint64 vAddr, int flag);
int      kvm_clrflag (pgtab_t *pgtab, uint64 vAddr, int flag);
uint64   kvm_pageaddr (pgtab_t *pgtab, uint64 vAddr);
uint64   kvm_phyaddr (pgtab_t *pgtab, uint64 vAddr);
int      kvm_map     (pgtab_t *pgtab, uint64 vAddr, uint64 pAddr, uint64 sz, int flag);
void     uvm_unmap   (pgtab_t *pgtab, uint64 vAddr, uint64 npages, bool free);
uint64   uvm_alloc   (pgtab_t *pgtab, uint64 start_addr, uint64 end_addr, int flag);
uint64   uvm_free    (pgtab_t *pgtab, uint64 oldsz, uint64 newsz);
void     uvm_destroy (pgtab_t *pgtab, uint64 size);
int      uvm_copyout (pgtab_t *pgtab, uint64 dst_va, char *src, uint64 len);
int      uvm_copyin  (pgtab_t *pgtab, char *dst, uint64 src_va, uint64 len);
int      uvm_copy    (pgtab_t *destPage, pgtab_t *srcPage, uint64 sz, bool alloc);

/******************** trap ********************/
void init_trap     (void);
void trap_userfunc (void);
void trap_userret  (void);
void kerneltrap    (void);


/******************** test ********************/
void init_selfdetect (void);


/******************** proc ********************/
int  proc_applypid  (void);
void proc_wakeup    (struct ProcCB *pcb);
int  proc_killstate (struct ProcCB *pcb);
void do_scheduler (void);
void do_defuncter (void);
void do_switch    (void);
void do_yield     (void);
void do_suspend   (void *obj);
void do_resume    (void *obj);
int  do_fork      (void);
int  do_wait      (int *code);
void do_exit      (int state);
int  do_kill      (int pid);
int  do_sleep     (int ms);
void init_proc    (void);
struct CpuCB  *getCpuCB (void);
void destroy_kthread (struct ProcCB *pcb);
struct ProcCB *create_kthread (char *name, void(*entry)(void));
#define getCpuID() r_tp()


/******************** pcb ********************/
void pcb_dump (void);
int  pcb_free (struct ProcCB *pcb);
int  proc_free_pgtab (pgtab_t *pgtab, uint64 size);
pgtab_t *proc_alloc_pgtab (struct ProcCB *pcb);
struct ProcCB *getProcCB  (void);
struct ProcCB *pcb_alloc  (void);
struct ProcCB *pcb_lookup (int pid);


/******************** plic ********************/
void init_plic     (void);
void init_plichart (void);
int  plic_claim    (void);
void plic_complete (int irq);


/******************** syscall ********************/
void do_syscall (void);


/******************** switch ********************/
void kswitch_to (struct Context *old, struct Context *new);


/******************** Port **********************/
void k_enable_all_interrupt (void);
void kPortDisableInterrupt (void);
void kPortEnableInterrupt (void);
#define kDISABLE_INTERRUPT()    kPortDisableInterrupt()
#define kENABLE_INTERRUPT()     kPortEnableInterrupt()


/******************** error *********************/
void ErrPrint (char *fmt, ...);


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


/******************** init **********************/
void init_main (void);
void idle_main (void);


/******************** timer *********************/
void init_timer (void);
void timer_del  (struct Timer *timer);
void timer_run  (void);
struct Timer *timer_add (struct ProcCB *pcb, int expires);


/******************** exec **********************/
int do_exec(char *path, char *argv[]);


/******************** device ********************/
void init_dev (void);
void dev_free (struct Device *dev);
void dev_register (struct Device *dev);
void dev_unregister (struct Device *dev);
void dev_put (struct Device *dev);
struct Device *dev_alloc (const char *name);
struct Device *dev_get (const char *name);


#endif
