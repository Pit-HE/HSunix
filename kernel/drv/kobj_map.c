/*
 * 内核对象哈希散列表管理模块
 */
#include "kobj_map.h"


struct kobj_map
{
    struct probe
    {
        struct probe    *next;
        dev_t           dev;
        unsigned long   range;
        kobj_probe_t    *get;
        int (*lock)(dev_t, void *);
        void            *data;
    } *probes[255];
};


int kobj_map(struct kobj_map *domain, dev_t dev, unsigned long range,
        kobj_probe_t *probe, int (*lock)(dev_t, void *), void *data)
{
    return 0;
}

void kobj_unmap (struct kobj_map *domain, dev_t dev, unsigned long range)
{

}

struct kobject *kobj_lookup (struct kobj_map *domain, dev_t dev, int *index)
{
    return NULL;
}

struct kobj_map *kobj_map_init (kobj_probe_t *base_probe)
{
    return NULL;
}

