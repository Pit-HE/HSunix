
#ifndef __UCORE_DEFS_H__
#define __UCORE_DEFS_H__


void error (void);

/** uart **/
void uartinit (void);
void uartputc_async(int);
void uartputc_sync(int);
int  uartgetc_loop(void);
int  uartgetc_intr(void);
void uartintr (void);

/** console **/
void console_init (void);
int  console_wCmd (char *src, int n);
void console_wChar (char *src);
int  console_rCmd (char *src);
int  console_rChar (void);

/** string **/
void *memset (void *s, int c, uint n);
void *memmove(void *dst, const void *src, uint n);
int strlen (const char *st);
char *strcpy (char *dest, const char *src);

/** kalloc **/
void *kalloc (void);
void  kfree  (void *pa);
void  kinit  (void);

/** vm **/
void    kvm_init (void);
void    kvm_enable (void);
uint64  kvm_phyaddr (pagetable_t pagetable, uint64 va);
void    kvm_map (pagetable_t pagetable, uint64 va, uint64 pa, uint64 sz, int flag);
void    uvm_unmap (pagetable_t pagetable, uint64 va, uint64 npages, bool free);
pagetable_t uvm_create (void);
uint64  uvm_alloc (pagetable_t pagetable, uint64 oldaddr, uint64 newaddr, int flag);
uint64  uvm_free (pagetable_t pagetable, uint64 oldsz, uint64 newsz);
void    uvm_destroy (pagetable_t pagetable, uint64 sz);
int     uvm_copy (pagetable_t destPage, pagetable_t srcPage, uint64 sz, bool alloc);
int     copyout (pagetable_t pagetable, uint64 dstva, char *src, uint64 len);
int     copyin (pagetable_t pagetable, char *dst, uint64 srcva, uint64 len);



#endif
