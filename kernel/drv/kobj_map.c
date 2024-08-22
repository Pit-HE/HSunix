/*
 * 内核对象哈希散列表管理模块
 */
#include "defs.h"
#include "kobj_map.h"


#define DEF_RANGE_VALUE     0xFFFF

/* 哈希散列表描述符
 * ( 管理设备号，通过设备号获取指定对象的 kobject )
 */
struct kobj_map
{
    struct probe
    {
        struct probe    *next;      /* 单向链表指针 */
        dev_t           dev;        /* 记录存储的设备号 */
        unsigned long   range;      /* 次设备号的数量 */
        kobj_probe_t    *get;       /* 由用户注册的，通过设备号获取对于 kobject 对象的接口 */
        int (*lock)(dev_t, void *); /* 由用户注册的，给目标对象上锁的接口 */
        void            *data;      /* 私有数据域 */
    } *probes[255];
};


/* 创建指定数量的 probe 结构体，并将其放入所属的哈希散列表中 */
int kobj_map(struct kobj_map *domain, dev_t dev, unsigned long range,
        kobj_probe_t *probe, int (*lock)(dev_t, void *), void *data)
{
    /* 计算要操作的主设备号数量 */
    unsigned major_num = MAJOR(dev + range -1) - MAJOR(dev) + 1;
    /* 次设备号作为索引 */
    unsigned index = MINOR(dev);
    struct probe *pb, *ptr;
    int i;

    /* 限制操作的主设备号数量 */
    if (major_num > 255)
        major_num = 255;

    /* 分配指定数量的 probe 对象 */
    pb = kalloc(sizeof(struct probe) * major_num);
    if (pb == NULL)
        return -1;

    /* 初始化分配的所有 probe 对象 */
    for (i=0; i<major_num; i++, pb++)
    {
        pb->next    = NULL;
        pb->dev     = dev;
        pb->range   = range;
        pb->get     = probe;
        pb->lock    = lock;
        pb->data    = data;
    }

    /* 将每个 probe 插入到其所属的管理链表中 */
    for (i=0, pb -= major_num; i<major_num; i++, pb++, index++)
    {
        /* 获取要操作的数组成员 */
        ptr = domain->probes[index % 255];

        /* 遍历该数组成员的管理链表 */
        while (ptr->next && ptr->next->range < range)
            ptr = ptr->next;

        /* 将 probe 对象插入合适的链表位置 */
        pb->next = ptr->next;
        ptr->next = pb;
    } 

    return 0;
}

/* 从哈希散列表移除指定的多个目标 */
void kobj_unmap (struct kobj_map *domain, dev_t dev, unsigned long range)
{
    /* 计算要操作的主设备号数量 */
    unsigned major_num = MAJOR(dev + range -1) - MAJOR(dev) + 1;
    /* 次设备号作为索引 */
    unsigned index = MINOR(dev);
    struct probe *obj = NULL;
    struct probe *pb, *ptr;
    int i;

    /* 限制操作的主设备号数量 */
    if (major_num > 255)
        major_num = 255;

    /* 寻找主设备号对应的所有哈希散列表数组成员 */
    for (i=0; i<major_num; i++, index++)
    {
        /* 遍历数组成员管理的链表 */
        for(pb = domain->probes[index % 255]; pb->next; pb = pb->next)
        {
            ptr = pb->next;
            /* 确认是否为所寻找的目标 */
            if ((ptr->dev == dev) && (ptr->range == range))
            {
                /* 将所寻目标从链表移除 */
                pb->next = ptr->next;

                /* 处理在 kobj_map 中一次记录多个跨主设备号的 probe */
                if (obj == NULL)
                    obj = ptr;
                break;
            }
        }
    }
    kfree(obj);
}

/* 从散列表中查找设备号对应的目标对象 */
struct kobject *kobj_lookup (struct kobj_map *domain, dev_t dev, int *index)
{
    struct probe *pb;
    struct kobject *kobj;
    unsigned long range = DEF_RANGE_VALUE;

    /* 遍历哈希散列表数组成员管理的链表 */
    for (pb = domain->probes[MAJOR(dev) % 255]; pb; pb = pb->next)
    {
        /* 确认遍历到的目标是否为所寻找的对象 */
        if ((pb->dev > dev) || (pb->dev + pb->range - 1 < dev))
            continue;
        if (pb->range - 1 >= range)
            break;

        /* 更新数据 */
        range  = pb->range - 1;
        *index = dev - (pb->dev);

        /* 执行 lock，给目标对象上锁 */
        if (pb->lock && (pb->lock(pb->dev, pb->data) < 0))
            continue;

        /* 执行 probe 函数获取目标对象的 kobject */
        if (pb->get == NULL)
            return NULL;
        kobj = pb->get(dev, index, pb->data);
        if (kobj == NULL)
            break;
        return kobj;
    }
    return NULL;
}

/* 初始化传入的哈希散列表 */
struct kobj_map *kobj_map_init (kobj_probe_t *base_probe)
{
    int i;
    struct probe *pb;
    struct kobj_map *map;

    if (base_probe == NULL)
        return NULL;

    /* 获取所需的内存资源 */
    pb = kalloc(sizeof(struct probe));
    map = kalloc(sizeof(struct kobj_map));
    if ((pb == NULL) || (map == NULL))
    {
        kfree(pb);
        kfree(map);
        return NULL;
    }

    /* 初始化 probe 参数 */
    kmemset(pb, 0, sizeof(struct probe));
    pb->dev = 1;
    pb->range = DEF_RANGE_VALUE;
    pb->get = base_probe;

    /* 初始化哈希散列表 */
    kmemset(map, 0, sizeof(struct kobj_map));
    for (i=0; i<255; i++)
        map->probes[i] = pb;
    return map;
}

