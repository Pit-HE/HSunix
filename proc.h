
// Saved registers for kernel context switches.
/* risc-v 的核心寄存器集 */
struct context
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
};

// Per-CPU state.
/* cpu 控制块 */
struct cpu
{
  /* 记录占用当前 CPU 的进程 */
  struct proc *proc;          // The process running on this cpu, or null.
  /* CPU 调度器进程的上下文，作为进程切换的过渡 */
  struct context context;     // swtch() here to enter scheduler().
  /* 开关中断的嵌套计数 */
  int noff;                   // Depth of push_off() nesting.
  int intena;                 // Were interrupts enabled before push_off()?
};

extern struct cpu cpus[NCPU];

// per-process data for the trap handling code in trampoline.S.
// sits in a page by itself just under the trampoline page in the
// user page table. not specially mapped in the kernel page table.
// uservec in trampoline.S saves user registers in the trapframe,
// then initializes registers from the trapframe's
// kernel_sp, kernel_hartid, kernel_satp, and jumps to kernel_trap.
// usertrapret() and userret in trampoline.S set up
// the trapframe's kernel_*, restore user registers from the
// trapframe, switch to the user page table, and enter user space.
// the trapframe includes callee-saved user registers like s0-s11 because the
// return-to-user path via usertrapret() doesn't return through
// the entire kernel call stack.
/* 中断上下文 */
struct trapframe
{
  /*   0 */ uint64 kernel_satp;   // kernel page table
  /*   8 */ uint64 kernel_sp;     // top of process's kernel stack
  /*  16 */ uint64 kernel_trap;   // usertrap()
  /*  24 */ uint64 epc;           // saved user program counter
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
};

enum procstate
{
  UNUSED,   /* 空闲 */
  USED,     /* 刚被分配 */
  SLEEPING, /* 休眠 */
  RUNNABLE, /* 就绪 */
  RUNNING,  /* 正在执行 */
  ZOMBIE    /* 等待退出 */
};

// Per-process state
/* 进程控制块 */
struct proc
{
  // p->lock must be held when using these:
  enum procstate state;        // Process state
  void *chan;                  // If non-zero, sleeping on chan   (指向导致进程进入休眠的对象)
  int killed;                  // If non-zero, have been killed   (标记进程是否已经被杀死)
  /* 记录子进程退出时要返回给父进程的状态 */
  int xstate;                  // Exit status to be returned to parent's wait
  int pid;                     // Process ID

  // wait_lock must be held when using this:
  struct proc *parent;         // Parent process

  // these are private to the process, so p->lock need not be held.
  uint64 kstack;               // Virtual address of kernel stack (进程内核栈的虚拟地址)
  uint64 sz;                   // Size of process memory (bytes)  (进程占用的内存大小)
  pagetable_t pagetable;       // User page table                 (进程所属的用户页表)
  struct trapframe *trapframe; // data page for trampoline.S      (中断上下文(记录完整的 cpu 工作信息))
  struct context context;      // swtch() here to run process     (用于进程切换的上下文信息)
  struct file *ofile[NOFILE];  // Open files                      (存储文件描述符的指针数组)
  struct inode *cwd;           // Current directory               (当前工作路径)
  char name[16];               // Process name (debugging)
};
