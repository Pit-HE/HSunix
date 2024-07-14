/*
 * 内核管道功能模块的实现代码
 */
#include "defs.h"
#include "fcntl.h"
#include "pipe.h"
#include "fs.h"


int pipealloc (struct Inode **rNode, struct Inode **wNode)
{
    struct pipe_t *pipe = NULL;

    /* 获取管道对象所需的内存资源 */
    pipe = (struct pipe_t *)kalloc(sizeof(struct pipe_t));
    if (pipe == NULL)
        goto _err_pipealloc;

    /* 获取循环缓冲区的 buf 内存 */
    pipe->buf = (char *)kalloc(PIPE_BUF_SIZE);
    if (pipe->buf == NULL)
        goto _err_pipealloc;

    /* 创建能够读管道数据的文件节点 */
    *rNode = inode_getpipe(pipe, O_RDONLY, S_IRWXU);
    if (rNode == NULL)
        goto _err_pipealloc;

    /* 创建能够写管道数据的文件节点 */
    *wNode = inode_getpipe(pipe, O_WRONLY, S_IRWXU);
    if (wNode == NULL)
        goto _err_pipealloc;

    /* 初始化管道结构体 */
    kRingbuf_init(&pipe->rb, pipe->buf, PIPE_BUF_SIZE);
    pipe->rflag = TRUE;
    pipe->wflag = TRUE;

    return 0;
_err_pipealloc:
    if (pipe->buf)    
        kfree(pipe->buf);
    if (pipe)   
        kfree(pipe);
    if (*rNode) 
        kfree(*rNode);
    return -1;
}

int pipeclose (struct pipe_t *pipe, bool flag)
{
    if (pipe == NULL)
        return -1;

    if (flag)
    {
        /* 关闭管道的写端 */
        pipe->wflag = FALSE;
        do_resume(&pipe->wflag);
    }
    else
    {
        /* 关闭管道的读端 */
        pipe->rflag = FALSE;
        do_resume(&pipe->rflag);
    }

    /* 管道已经全部关闭 */
    if (FALSE == (pipe->wflag | pipe->rflag))
    {
        kfree(pipe->buf);
        kfree(pipe);
    }

    return 0;
}

int piperead (struct pipe_t *pipe, uint64 buf, uint n)
{
    int ret;
    struct ProcCB *pcb = getProcCB();

    /* 当缓冲区空间不足时，循环休眠等待缓冲区有足够大小*/
    while(n != kRingbuf_rDataSize(&pipe->rb))
    {
        if (0 != proc_killstate(pcb))
            return -1;
        /* 若没有了写端，则将缓冲区剩余数据读取完 */
        if (pipe->wflag == FALSE)
            break;

        do_suspend(&pipe->rflag);
    }

    /* 将数据写入缓冲区 */
    ret = kRingbuf_put(&pipe->rb, (char *)buf, n);
    /* 唤醒等待读取的进程 */ 
    do_resume(&pipe->wflag);

    return ret;
}

int pipewrite (struct pipe_t *pipe, uint64 buf, uint n)
{
    uint cnt = 0;
    char *addr = (char *)buf;
    struct ProcCB *pcb = getProcCB();

    if ((pipe == NULL) || (buf == 0) || (n == 0))
        return -1;

    /* 确保数据全部写入缓冲区 */
    while (cnt < n)
    {
        /* 若没有了读端，则数据没有写入的意义 */
        if (pipe->rflag == FALSE)
            return -1;
        /* 确认当前进程是否还活着 */
        if (0 == proc_killstate(pcb))
            return -1;
        
        /* 处理管道没有剩余空间可写的情况 */
        if (0 == KRingbuf_wDataSize(&pipe->rb))
        {
            do_resume(&pipe->rflag);
            do_suspend(&pipe->wflag);
            continue;
        }

        /* 将数据写入管道的缓冲区内 */
        cnt += kRingbuf_put(&pipe->rb, addr + cnt, n - cnt);
    }

    /* 唤醒读端搬运数据 */
    do_resume(&pipe->rflag);
    return 0;
}
