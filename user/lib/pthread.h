
#ifndef __PTHREAD_H__
#define __PTHREAD_H__


#include "types.h"
#include "list.h"


/* 调度属性 */
typedef unsigned int sched_param;
typedef unsigned int pthread_t;


/* 线程的状态 */
enum Threadstate
{
  IDLE,     /* 空闲 */
  USED,     /* 刚被分配 */
  SUSPEND,  /* 挂起 */
  SLEEPING, /* 休眠 */
  READY,    /* 就绪 */
  RUNNING,  /* 正在执行 */
  EXITING   /* 等待退出 */
};
/* 线程切换时的上下文 */
struct Context  //processSwitchContext
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

/* 线程属性 
 * ( 目前未提供线程属性的设置功能 )
 */
typedef struct
{
    int     detachstate;     //线程的分离状态
    int     schedpolicy;     //线程调度策略
    sched_param schedparam;  //线程的调度参数
    int     inheritsched;    //线程的继承性
    int     scope;           //线程的作用域
    size_t  guardsize;       //线程栈末尾的警戒缓冲区大小
    int     stackaddr_set;
    void   *stackaddr;       //线程栈的位置
    size_t  stacksize;       //线程栈的大小
}pthread_attr_t;


/* 线程控制块 */
typedef struct
{
    void *(*entry)(void *);
    ListEntry_t          list;
    struct Context       context;
    pthread_t            tid;
    enum Threadstate     stat;
    void                *exitval;
    const pthread_attr_t *attr;
    void                *arg;
    unsigned int         sleep;
    void                *stack;
}ThreadCB;


/* 外部接口 */
int  pthread_exit   (void *retval);
int  pthread_join   (pthread_t thread, void **retval);
int  pthread_sleep  (int ms);
int  pthread_create (pthread_t *thread, const pthread_attr_t *attr, 
        void *(*start_routine)(void *), void *arg);


#endif
