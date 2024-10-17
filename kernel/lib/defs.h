
#ifndef __UCORE_DEFS_H__
#define __UCORE_DEFS_H__


#include "types.h"
#include "riscv.h"
#include "tinyprintf.h"


/* 通过结构体成员获取结构体首地址 */
#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))


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


/******************** alloc_page ********************/
void *alloc_page (void);
void  free_page  (void *pa);
void *kalloc     (unsigned long size);
void  kfree      (void *obj);
void  init_slab  (void);


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
int      copy_to_user   (uint64 dst_va, char *src, uint64 len);
int      copy_from_user (char *dst, uint64 src_va, uint64 len);


/******************** trap ********************/
void init_trap     (void);
void trap_userfunc (void);
void trap_userret  (void);
void kerneltrap    (void);


/******************** test ********************/
void init_selfdetect (void);


/******************** plic ********************/
void init_plic     (void);
void init_plichart (void);
int  plic_claim    (void);
void plic_complete (int irq);


/******************** syscall ********************/
void do_syscall (void);


/******************** Port **********************/
void k_enable_all_interrupt (void);
void kPortDisableInterrupt (void);
void kPortEnableInterrupt (void);
#define kDISABLE_INTERRUPT()    kPortDisableInterrupt()
#define kENABLE_INTERRUPT()     kPortEnableInterrupt()


/******************** kerror *********************/
void ErrPrint (char *fmt, ...);


/******************** init **********************/
void init_main (void);
void idle_main (void);


/******************** exec **********************/
int do_exec(char *path, char *argv[]);


#endif
