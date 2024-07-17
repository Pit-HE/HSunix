/*
 * 内核消息队列模块
 */
#include "defs.h"


/* 消息队列的数据节点，存放要传递的数据 */
typedef struct 
{
    /* 要传递的数据大小 */
    uint        size;
    /* 存放要传递的数据 */
    void       *buf;
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

static ListEntry_t MsgList;
static uint msg_idtoken = 0;


void init_msg (void)
{
    msg_idtoken = 1;
    list_init(&MsgList);
}


int msgget (int key, int msgflg)
{
    msg_queue  *queue;

    queue = (msg_queue *)kalloc(sizeof(msg_queue));

    queue->num  = 0;
    queue->key  = key;
    queue->flag = msgflg;
    queue->id   = msg_idtoken++;

    list_init(&queue->msg);
    list_init(&queue->self);

    list_add(&MsgList, &queue->self);

    return 0;
}

/* 通过消息队列发送一个数据 */
int msgsnd (int msqid, void *msgp, uint msgsz,
        int msgflg)
{
    msg_queue *queue = NULL;
    ListEntry_t *ptr = NULL;
    msg_data *msg = NULL;
    void *buf = NULL;

    /* 申请存放数据的内存空间 */
    buf = kalloc(msgsz);
    kmemcpy(buf, msgp, msgsz);

    /* 存放要传递的数据 */     
    msg->size = msgsz;
    msg->buf  = buf;
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

int msgrcv (int msqid, void *msgp, uint msgsz,
        int msgtyp, int msgflg)
{
    msg_queue *queue = NULL;
    ListEntry_t *ptr = NULL;
    msg_data *msg = NULL;
    uint len;

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

        /* 获取该数据节点 */
        msg = list_container_of(queue->msg.next, msg_data, list);
        list_del_init(&msg->list);

        /* 读取数据节点内的信息 */
        len = (msgsz > msg->size) ? msg->size:msgsz;
        kmemcpy(msgp, msg->buf, len);

        /* 释放该数据节点占用的内存空间 */
        kfree(msg->buf);
        kfree(msg);
        return len;
    }
    return -1;
}

/* 暂不支持控制功能 */
int msgctl (int msqid, int cmd, void *uptr)
{
    return 0;
}
