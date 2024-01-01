
#ifndef __UCORE_DEFS_H__
#define __UCORE_DEFS_H__


/** uart **/
void uartinit(void);
void uartputc_async(int);
void uartputc_sync(int);
int uartgetc_loop(void);
int uartgetc_intr(void);
void uartintr (void);

/** console **/
void console_init (void);
int console_wCmd (char *src, int n);
void console_wChar (char *src);
int console_rCmd (char *src);
int console_rChar (void);

/** string **/
void *memset (void *s, int c, uint n);
void *memmove(void *dst, const void *src, uint n);
int strlen (const char *st);
char *strcpy (char *dest, const char *src);

/** kalloc **/
void *kalloc (void);
void kfree (void *pa);
void kinit (void);

/** vm **/
void    kvm_init (void);
void    kvm_enable (void);
int     mappages (pagetable_t, uint64, uint64, uint64, int);
uint64  kvm_phyaddr (pagetable_t, uint64);
void    kvm_map (pagetable_t, uint64, uint64, uint64, int);
void    freepages (pagetable_t);
void    uvm_unmap (pagetable_t, uint64, uint64, bool);
pagetable_t uvm_create (void);
uint64  uvm_alloc (pagetable_t, uint64, uint64, int);
uint64  uvm_free (pagetable_t, uint64, uint64);
void    uvm_destroy (pagetable_t, uint64);
int     uvm_copy (pagetable_t, pagetable_t, uint64, bool);
int     copyout (pagetable_t, uint64, char *, uint64);
int     copyin (pagetable_t, char *, uint64, uint64);

#endif
