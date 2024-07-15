/*
 * 内核管道功能模块的实现代码
 */
#include "defs.h"
#include "fcntl.h"
#include "file.h"
#include "pipe.h"


/* 管道的数据读取接口 */
static int piperead (struct File *file, void *buf, uint n)
{
    uint cnt = 0;
    char *addr = (char *)buf;
    struct pipe_t *pipe = NULL;
    struct ProcCB *pcb = getProcCB();

    if ((file == NULL) || (buf == NULL) || (n == 0))
        return -1;
    pipe = (struct pipe_t *)file->inode->data;

    /* 确保能够读取到指定长度的数据 */
    while (cnt < n)
    {
        /* 确认当前进程是否还活着 */
        if (proc_killstate(pcb))
            return -1;
        
        /* 处理管道没有数据可以读取的情况 */
        if (0 == KRingbuf_rDataSize(&pipe->rb))
        {
            do_resume(&pipe->wflag);
            do_suspend(&pipe->rflag);
            continue;
        }

        /* 从管道内获取数据写入缓冲区 */
        cnt += kRingbuf_get(&pipe->rb, addr + cnt, n - cnt);

        /* 若没有写端，则退出数据的读取操作 */
        if (pipe->wflag == FALSE)
            return -1;
    }
    /* 唤醒读端搬运数据 */
    do_resume(&pipe->wflag);

    return cnt;
}

/* 管道的数据写入接口 */
static int pipewrite (struct File *file, void *buf, uint n)
{
    uint cnt = 0;
    char *addr = (char *)buf;
    struct pipe_t *pipe = NULL;
    struct ProcCB *pcb = getProcCB();

    if ((file == NULL) || (buf == NULL) || (n == 0))
        return -1;
    pipe = (struct pipe_t *)file->inode->data;

    /* 确保数据全部写入缓冲区 */
    while (cnt < n)
    {
        /* 若没有了读端，则数据没有写入的意义 */
        if (pipe->rflag == FALSE)
            return -1;
        /* 确认当前进程是否还活着 */
        if (proc_killstate(pcb))
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

    return cnt;
}

/* 管道的关闭接口 */
static int pipeclose (struct File *file)
{
    struct pipe_t *pipe = NULL;

    if (file == NULL)
        return -1;
    pipe = (struct pipe_t *)file->inode->data;

    if (file->flags == O_WRONLY)
    {
        /* 关闭管道的写端 */
        pipe->wflag = FALSE;
        do_resume(&pipe->wflag);
    }
    else if (file->flags == O_RDONLY)
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

/* 将管道功能注册成内核文件设备 */
struct FileOperation pipe_opt = 
{
    .read  = piperead,
    .write = pipewrite,
    .close = pipeclose,
};


/* 管道对象的创建接口 */
int pipealloc (struct File *rfile, struct File *wfile)
{
    struct Inode *rnode = NULL;
    struct Inode *wnode = NULL;
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
    rnode = inode_getpipe(pipe, O_RDONLY, S_IRWXU);
    if (rnode == NULL)
        goto _err_pipealloc;

    /* 创建能够写管道数据的文件节点 */
    wnode = inode_getpipe(pipe, O_WRONLY, S_IRWXU);
    if (wnode == NULL)
        goto _err_pipealloc;

    /* 初始化管道结构体 */
    kRingbuf_init(&pipe->rb, pipe->buf, PIPE_BUF_SIZE);
    pipe->rflag = TRUE;
    pipe->wflag = TRUE;

    /* 初始化读端的文件描述符 */
    rfile->inode = rnode;
    rfile->inode->data = pipe;
    rfile->ref  += 1;
    rfile->fops  = &pipe_opt;
    rfile->flags = O_RDONLY;
    rfile->ditem = NULL;

    /* 初始化写端的文件描述符 */
    wfile->inode = wnode;
    wfile->inode->data = pipe;
    wfile->ref  += 1;
    wfile->fops  = &pipe_opt;
    wfile->flags = O_WRONLY;
    wfile->ditem = NULL;

    return 0;
_err_pipealloc:
    if (pipe->buf)    
        kfree(pipe->buf);
    if (pipe)   
        kfree(pipe);
    if (rnode) 
        kfree(rnode);
    return -1;
}
