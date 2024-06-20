
#ifndef __PROC_H___
#define __PROC_H___

#include "riscv.h"
#include "list.h"
#include "file.h"

/* Process state */
enum Procstate
{
  IDLE,     /* 空闲 */
  USED,     /* 刚被分配 */
  SUSPEND,  /* 挂起 */
  SLEEPING, /* 休眠 */
  READY,    /* 就绪 */
  RUNNING,  /* 正在执行 */
  EXITING   /* 等待退出 */
};

// Saved registers for kernel context switches.
typedef struct processSwitchContext
{
  uint64 ra;
  uint64 sp;

  // callee-saved
  uint64 s0;
  uint64 s1;
  uint64 s2;
  uint64 s3;
  uint64 s4;
  uint64 s5;
  uint64 s6;
  uint64 s7;
  uint64 s8;
  uint64 s9;
  uint64 s10;
  uint64 s11;
}Context;

typedef struct trapStackFrame
{
  /*   0 */ uint64 epc;           // saved user program counter
  /*   8 */ uint64 kernel_satp;   // kernel page table
  /*  16 */ uint64 kernel_sp;     // top of process's kernel stack
  /*  24 */ uint64 kernel_trap;   // usertrap()
  /*  32 */ uint64 kernel_hartid; // saved kernel tp
  /*  40 */ uint64 ra;
  /*  48 */ uint64 sp;
  /*  56 */ uint64 gp;
  /*  64 */ uint64 tp;
  /*  72 */ uint64 t0;
  /*  80 */ uint64 t1;
  /*  88 */ uint64 t2;
  /*  96 */ uint64 s0;
  /* 104 */ uint64 s1;
  /* 112 */ uint64 a0;
  /* 120 */ uint64 a1;
  /* 128 */ uint64 a2;
  /* 136 */ uint64 a3;
  /* 144 */ uint64 a4;
  /* 152 */ uint64 a5;
  /* 160 */ uint64 a6;
  /* 168 */ uint64 a7;
  /* 176 */ uint64 s2;
  /* 184 */ uint64 s3;
  /* 192 */ uint64 s4;
  /* 200 */ uint64 s5;
  /* 208 */ uint64 s6;
  /* 216 */ uint64 s7;
  /* 224 */ uint64 s8;
  /* 232 */ uint64 s9;
  /* 240 */ uint64 s10;
  /* 248 */ uint64 s11;
  /* 256 */ uint64 t3;
  /* 264 */ uint64 t4;
  /* 272 */ uint64 t5;
  /* 280 */ uint64 t6;
}Trapframe;



// Per-process state
/* 进程控制块 */
typedef struct processControlBlock
{
  enum Procstate state;                 // Process state
  struct processControlBlock *parent;   // Parent process

  void         *pendObj;          // If non-zero, sleeping on special object
  uint          killState;        // If non-zero, have been killed
  uint          exitState;        // Exit status to be returned to parent's wait
  uint          pid;              // Process ID

  ListEntry_t   regist;           // 创建进程时用于注册到内核的 kRegistList 链表
  ListEntry_t   list;             // Manage process state switch

  struct File **fdTab;            // 文件描述符的指针数组
  uint          fdCnt;            // 记录当前文件描述符数组的长度(可以动态变化)
  char         *cwd;              // 进程的工作路径

  uint64        stackAddr;        // Virtual address of kernel stack
  uint64        stackSize;        // Virtual address of kernel stack size
  uint64        memSize;          // Size of process memory (bytes)
  pgtab_t  *pageTab;          // User page table
  Trapframe    *trapFrame;        // data page for trampoline.S
  Context       context;          // switch_to() here to run process
  char          name[20];         // Process name (debugging)
}ProcCB;

typedef struct cpuControlBlock
{
  ProcCB   *proc;                 // The process running on this cpu, or null.
  Context   context;              // switch_to() here to enter do_scheduler().
  int       intrOffNest;          // Depth of push_off() nesting.
  int       intrOldState;         // Were interrupts enabled before push_off()?
}CpuCB;


#endif
