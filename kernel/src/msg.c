/*
 * 内核消息队列模块
 */
#include "defs.h"
#include "fcntl.h"
#include "list.h"
#include "ringbuff.h"
#include "proc.h"
#include "pcb.h"


/* 消息内容节点，记录消息的类型与内容
 * ( 用于数据的检索 )
 */
typedef struct
{
    long    type;   /* 消息类型 */
    void   *mtext;  /* 消息正文，可以是任意类型 */
}msg_info;

/* 消息队列的数据节点，存放要传递的数据 */
typedef struct 
{
    /* 要传递的数据大小 */
    uint        size;
    /* 存放要传递的数据 */
    msg_info    info;
    /* 与其他数据节点组成链表 */
    ListEntry_t list;
}msg_data;

/* 消息队列，管理要传递的数据节点(msg_data) */
typedef struct 
{
    /* 用于做区分标识的键值 */
    int         key;
    /* 数据节点的 ID */
    int         id;
    /* 操作标记 */
    int         flag;
    /* 消息节点的数量 */
    int         num;
    /* 与其他 msg_queue 组成链表 */
    ListEntry_t self;
    /* 指向被管理的消息节点 */
    ListEntry_t msg;
}msg_queue;


static uint msg_idtoken;
static ListEntry_t MsgList;

#define abs(n)  (((n) < 0) ? -(n):(n))


void init_msg (void)
{
    msg_idtoken = 1;
    list_init(&MsgList);
}


int k_msgget (int key, int msgflg)
{
    bool excl = FALSE;
    ListEntry_t *ptr = NULL;
    msg_queue *queue = NULL;

    list_for_each(ptr, &MsgList)
    {
        queue = (msg_queue *)
            list_container_of(ptr, msg_queue, self);
        if (queue->key == key)
        {
            excl = TRUE;
            break;
        }
    }

    if (excl == TRUE)
    {
        /* 处理消息队列已经存在的情况 */
        if (msgflg & IPC_EXCL)
            return -1;
    }
    else if (msgflg & IPC_CREAT)
    {
        /* 处理消息队列不存在的情况 */
        queue = (msg_queue *)kalloc(sizeof(msg_queue));

        /* 初始化消息队列 */
        queue->num  = 0;
        queue->key  = key;
        queue->flag = msgflg;
        queue->id   = msg_idtoken++;
        list_init(&queue->msg);
        list_init(&queue->self);

        /* 将消息队列添加到链表 */
        list_add(&MsgList, &queue->self);
    }

    return queue->id;
}

/* 通过消息队列发送数据
 * ( 忽略 msgflg 的作用 )
 */
int k_msgsnd (int msqid, void *msgp, uint msgsz,
        int msgflg)
{
    void *buf = NULL;
    msg_data *msg = NULL;
    msg_queue *queue = NULL;
    ListEntry_t *ptr = NULL;
    msg_info *info = (msg_info *)msgp;

    /* 记录用户空间需要传递的数据 */
    buf = kalloc(msgsz);
    copy_from_user(buf, (uint64)info->mtext, msgsz);

    /* 存放要传递的数据 */
    msg = (msg_data *)kalloc(sizeof(msg_data));
    msg->size = msgsz;
    msg->info.type = info->type;
    msg->info.mtext = buf;
    list_init(&msg->list);

    /* 寻找存放数据的消息队列 */
    list_for_each(ptr, &MsgList)
    {
        queue = list_container_of(ptr, msg_queue, self);
        if (queue->id != msqid)
            continue;
        
        /* 将数据放入消息队列 */
        list_add_before(&queue->msg, &msg->list);
        queue->num += 1;

        /* 唤醒等待在队列上的进程 */
        do_resume(queue);
        return msgsz;
    }

    return -1;
}

/* 
 * 通过消息队列接收数据
 * ( 忽略 msgflg 的作用 )
 */
int k_msgrcv (int msqid, void *msgp, uint msgsz,
        int msgtyp, int msgflg)
{
    uint len;
    msg_data *msg = NULL;
    msg_queue *queue = NULL;
    ListEntry_t *ptr = NULL;
    ListEntry_t *qtr = NULL;
    msg_info *info = (msg_info *)msgp;   

    /* 遍历模块内的所有消息队列节点 */
    list_for_each(ptr, &MsgList)
    {
        /* 判断该消息队列是否存在 */
        queue = list_container_of(ptr, msg_queue, self);
        if (queue->id != msqid)
            continue;

        /* 若消息队列内还未有数据，则让进程挂起等待 */
        if (queue->num <= 0)
            do_suspend(queue);
        queue->num -= 1;

        list_for_each(qtr, &queue->msg)
        {
            /* 获取该数据节点 */
            msg = list_container_of(qtr, msg_data, list);

            if (msgtyp > 0)
            {
                /* 查找指定消息 */
                if (msg->info.type != msgtyp)
                    continue;
            }
            else if (msgtyp < 0)
            {
                if (msg->info.type > abs(msgtyp))
                    continue;
            }
            len = (msgsz > msg->size) ? msg->size:msgsz;

            /* 将数据节点从链表内移除 */
            list_del_init(&msg->list);

            /* 读取数据节点内的信息 */
            info->type = msg->info.type;
            copy_to_user((uint64)info->mtext, msg->info.mtext, len);

            /* 释放该数据节点占用的内存空间 */
            kfree(msg->info.mtext);
            kfree(msg);
            return len;
        }
    }
    return -1;
}

/* 暂不支持控制功能 */
int k_msgctl (int msqid, int cmd, void *uptr)
{
    return 0;
}
