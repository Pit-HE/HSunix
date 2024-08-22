
#ifndef __KOBJ_MAP_H__
#define __KOBJ_MAP_H__


#include "types.h"


typedef struct kobject *kobj_probe_t(dev_t, int *, void *);
struct kobj_map;


int kobj_map(struct kobj_map *domain, dev_t dev, unsigned long range,
        kobj_probe_t *probe, int (*lock)(dev_t, void *), void *data);
void kobj_unmap (struct kobj_map *domain, dev_t dev, unsigned long range);
struct kobject *kobj_lookup (struct kobj_map *domain, dev_t dev, int *index);
struct kobj_map *kobj_map_init (kobj_probe_t *base_probe);


#endif
