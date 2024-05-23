/*
 * 设备管理模块，管理注册到系统中的所有设备对象
 * ( 标准的操作系统中，设备对象应该注册到文件系统中的指定路径，
 *   但是目前为了减少难度，就将设备独立出来单独操作 )
 */
#include "defs.h"
#include "device.h"


/* 以链表的形式记录登记到系统中的所有设备 */
ListEntry_t     gDevList;


void dev_init (void)
{
    list_init(&gDevList);
}

/* 申请设备对象的内存空间 */
struct Device *dev_alloc (const char *name)
{
    struct Device *dev;

    dev = (struct Device *)kalloc(sizeof(struct Device));
    if (dev == NULL)
        return NULL;
    kmemset (dev, 0, sizeof(struct Device));

    list_init (&dev->list);

    dev->magic = DEVICE_MAGIC;
    dev->name = (char *)name;

    return dev;
}

/* 释放已申请的设备对象 */
void dev_free (struct Device *dev)
{
    if (dev == NULL)
        return;
    if (dev->magic != DEVICE_MAGIC)
        return;
    if (dev->ref != 0)
        return;

    kfree(dev);
}

/* 将设备对象挂载到管理链表上 */
void dev_register (struct Device *dev)
{
    if (dev == NULL)
        return;
    if (dev->magic != DEVICE_MAGIC)
        return;

    list_add_after(&gDevList, &dev->list);
}

/* 注销已挂载到管理链表上的设备对象 */
void dev_unregister (struct Device *dev)
{
    if (dev == NULL)
        return;
    if (dev->magic != DEVICE_MAGIC)
        return;
    if (dev->ref != 0)
        return;

    list_del_init(&dev->list);
}

/* 通过名字获取已注册的设备对象 */
struct Device *dev_get (const char *name)
{
    struct Device   *dev;
    ListEntry_t     *ptr;

    if (name == NULL)
        return NULL;
    if (TRUE == list_empty(&gDevList))
        return NULL;

    /* 遍历当前注册的所有设备 */
    list_for_each(ptr, &gDevList)
    {
        dev = list_container_of(ptr, struct Device, list);

        /* 是否为所寻找的设备对象 */
        if (0 == kstrcmp(dev->name, name))
        {
            dev->ref += 1;
            return dev;
        }
    }

    return NULL;
}

/* 释放已获取的设备对象 */
void dev_put (struct Device *dev)
{
    if (dev == NULL)
        return;
    if (dev->magic != DEVICE_MAGIC)
        return;
    dev->ref -= 1;
    if (dev->ref != 0)
        return;

    dev_unregister(dev);
    dev_free(dev);
}
