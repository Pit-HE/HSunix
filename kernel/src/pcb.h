
#ifndef __PCB_H__
#define __PCB_H__


#include "types.h"
#include "riscv.h"


void pcb_dump (void);
int  pcb_free (struct ProcCB *pcb);
int  proc_free_pgtab (pgtab_t *pgtab, uint64 size);
pgtab_t *proc_alloc_pgtab (struct ProcCB *pcb);
struct ProcCB *getProcCB  (void);
struct ProcCB *pcb_alloc  (void);
struct ProcCB *pcb_lookup (int pid);


#endif
