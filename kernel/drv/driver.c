
#include "driver.h"


/* 注册驱动 */
int driver_register (struct device_driver *drv)
{
    struct device_driver *other;

    if (dev == NULL)
        return -1;

    /* 确认驱动还未注册 */
    other = driver_find (drv->name, drv->bus);
    if (other != NULL)
        return -1;

    list_init(&drv->device);
    return bus_add_driver(drv);
}

/* 注销驱动 */
int driver_unregister (struct device_driver *drv)
{
    if (drv)
        bus_remove_driver(drv);
}

/* 从指定总线中通过名字查找驱动对象 */
struct device_driver *driver_find (char *name, struct bus_type *bus)
{
    struct kobject *kobj;

    if ((name == NULL) || (bus == NULL))
        return NULL;

    /* 从总线的 kset 上获取指定驱动的内核对象 */
    kobj = kset_find_kobj(bus->driver, name);
    if (kobj == NULL)
        return NULL;

    /* 释放 kset_find_kobj 中递增的计数 */
    kobject_put(kobj);

    /* 获取驱动的结构体地址并返回 */
    return container_of(kobj, struct device_driver, kobj);
}

/* 从驱动的设备管理队列中获取指定的设备对象 */
struct device *driver_find_device (struct device_driver *drv,
            struct device *start, void *data,
            int (*match)(struct device *, void *))
{
    struct device *dev = NULL;
    ListEntry_t *ptr, *tmp;

    if (drv == NULL)
        return NULL;

    /* 遍历驱动管理设备的链表 */
    list_for_each_safe(ptr, tmp, &drv->device)
    {
        /* 获取设备的结构体首地址 */
        dev = container_of(ptr, struct device, drv_list);

        /* 检查该设备释放为所寻对象 */
        if (match(dev, data) > 0)
            break;
    }

    return dev;
}

/* 遍历驱动管理的所有子设备 */
struct device *driver_find_each_device (struct device_driver *drv,
            struct device *start, void *data,
            int (*fn)(struct device *, void *))
{
    int error;
    struct device *dev = NULL;
    ListEntry_t *ptr, *tmp;

    if (drv == NULL)
        return NULL;

    /* 遍历驱动管理设备的链表 */
    list_for_each_safe(ptr, tmp, &drv->device)
    {
        /* 获取设备的结构体首地址 */
        dev = container_of(ptr, struct device, drv_list);

        /* 检查该设备释放为所寻对象 */
        error = fn(dev, data);
        if (error <= 0)
            break;
    }

    return error;
}
