/*
 * 模拟 linux 的总线设备驱动模型的驱动模块
 * 
 * 1、参考了 Linux 的 2.6.11.12 和 5.10.120 的源码
 * 2、实现了总线模块最核心的基本功能，管理设备与驱动的增删查改
 */
#include "bus.h"


static void bus_release (struct kobject *kobj)
{

}
/* 总线子系统默认的属性 */
struct kobj_type bus_ktype = 
{
    .release = bus_release,
};
/* 总线子系统 */
struct kset *bus_subsys;


/*
 ********************************************************************
 *          总线模块的内部接口
 ********************************************************************
 */
/* 内部接口：遍历指定总线上的所有驱动对象 */
static int __bus_for_each_drv (struct bus_type *bus, void *data, 
                int (*fn)(struct device_driver *, void *))
{
    int retval = 0;
    ListEntry_t *ptr, *qtr;
    struct device_driver *drv;

    bus = get_bus(bus);
    if (bus == NULL)
        return -1;
    

    /*遍历总线上的所有设备对象*/
    list_for_each_safe(ptr, qtr, &bus->driver->list)
    {
        /*获取该设备对象的结构体首地址 */
        drv = container_of(ptr, struct device_driver, kobj.entry);

        /*执行回调，处理设备对象 */
        retval = fn(drv, data);
        if(retval <= 0)
            break;
    }

    put_bus(bus);
    return 0;
}

/* 内部接口：遍历指定总线上的所有设备对象 */
static int __bus_for_each_dev (struct bus_type *bus, void *data, 
                int (*fn)(struct device *, void *))
{
    int retval = 0;
    ListEntry_t *ptr, *qtr;
    struct device *dev;

    bus = get_bus(bus);
    if (bus == NULL)
        return -1;
    
    /* 遍历总线上的所有驱动 */
    list_for_each_safe(ptr, qtr, &bus->driver->list)
    {
        /* 获取该驱动对象的结构体首地址 */
        dev = container_of(ptr, struct device, bus_list);

        /* 执行回调，处理驱动对象 */
        retval = fn(dev, data);
        if (retval <= 0)
            break;
    }

    put_bus(bus);
    return 0;
}

/* 释放驱动上管理的所有设备 */
static void driver_detach (struct device_driver *drv)
{
    struct device *dev;
    ListEntry_t *ptr, *qtr;

    if (drv == NULL)
        return;
    
    /*遍历驱动中管理的所有设备对象 */
    list_for_each_safe(ptr, qtr, &drv->device)
    {
        /*获取设备对象的结构体首地址 */
        dev= container_of(ptr, struct device, drv_list);

        /*将设备从所属驱动链表移除 */
        device_release_driver(dev);
    }
}


 /*
 ********************************************************************
 *          操作总线的接口
 ********************************************************************
 */
/* 将总线对象注册到总线子系统 */
int  bus_register (struct bus_type *bus)
{
    /* 初始化总线内嵌的子系统 */
    kobject_setname(&bus->subsys.kobj, "%s", bus->name);
    bus->subsys.kobj.kset = bus_subsys;
    bus->subsys.kobj.ktype = &bus_ktype;
    kset_register(&bus->subsys);

    /* 初始化总线的设备管理链表 */
    bus->device = kset_create("device", NULL, &bus_subsys->kobj);
    if (bus->device != NULL)
        kset_register(bus->device);
    else
        goto bus_device_fail;
    
    /* 初始化总线的驱动管理链表 */
    bus->driver = kset_create("driver", NULL, &bus_subsys->kobj);
    if (bus->driver != NULL)
        kset_register(bus->driver);
    else
        goto bus_driver_fail;

    return 0;
bus_driver_fail:
    kset_unregister(bus->driver);
bus_device_fail:
    kset_unregister(bus->device);
    kset_unregister(&bus->subsys);
    return 0;
}

/* 注销总线子系统内的总线对象 */
int  bus_unregister (struct bus_type *bus)
{
    kset_unregister(bus->driver);
    kset_unregister(bus->device);
    kset_unregister(&bus->subsys);
    return 0;
}

/* 将设备添加到其所属的总线 */
int  bus_add_device (struct device *dev)
{
    struct bus_type *bus;

    /* 获取总线
     * (与 bus_remove_device 中的 put_bus 对应 )
     */
    bus = get_bus(dev->bus);
    if (bus == NULL)
        return -1;

    /* 将设备添加到其所属总线的管理链表中 */
    list_add_before(&dev->bus->device->list, &dev->bus_list);

    /* 为设备寻找匹配的驱动 */
    device_attach(dev);
    return 0;
}

/* 将驱动添加到其所属的总线 */
int bus_add_driver (struct device_driver *drv)
{
    int retval;
    struct bus_type *bus;

    /* 获取总线
     * (与 bus_remove_driver 中的 put_bus 对应 )
     */
    bus = get_bus(drv->bus);
    if (bus == NULL)
        return -1;
    
    /*设置驱动的内核对象的名字属性 */
    if (0 > kobject_setname(&drv->kobj, "%s", drv->name))
        return -1;

    /*将驱动注册到其所属总线的驱动管理糙表 */
    drv->kobj.kset = bus->driver;
    retval = kobject_register(&drv->kobj);

    /*为驱动寻找匹配的设备 */
    driver_attach(drv);
    return retval;
}

/* 将指定设备从总线上移除 */
void bus_remove_device (struct device *dev)
{
    if ((dev == NULL) || (dev->bus == NULL))
        return;

    device_release_driver(dev);
    list_del_init(&dev->bus_list);

    /* 释放总线
     * (与 bus_add_device 中的 get_bus 对应 )
     */
    put_bus(dev->bus);
}

/* 将指定驱动从总线上移除 */
void bus_remove_driver (struct device_driver *drv)
{
    if ((drv == NULL) || (drv->bus == NULL))
        return;
    
    driver_detach(drv);
    kobject_unregister(&drv->kobj);

    /* 释放总线
     * (与 bus_add_driver 中的 get_bus 对应 )
     */
    put_bus(drv->bus);
}

/* 释放总线 */
void put_bus (struct bus_type *bus)
{
    if (bus)
        kset_put(&bus->subsys);
}

/* 获取总线 */
struct bus_type *get_bus (struct bus_type *bus)
{
    if (bus)
        kset_get(&bus->subsys);
    return bus;
}

/* 遍历指定总线上的所有驱动对象 */
int bus_for_each_drv (struct bus_type *bus, struct device_driver *start,
            void *data, int (*fn)(struct device_driver *, void *))
{
    int retval;

    kDISABLE_INTERRUPT();
    retval = __bus_for_each_drv(bus, data, fn);
    kENABLE_INTERRUPT();

    return retval;
}

/* 遍历指定总线上的所有设备对象 */
int bus_for_each_dev (struct bus_type *bus, struct device *start,
            void *data, int (*fn)(struct device *, void *))
{
    int retval;

    kDISABLE_INTERRUPT();
    retval = __bus_for_each_dev(bus, data, fn);
    kENABLE_INTERRUPT();

    return retval;
}


/*
 ********************************************************************
 *          总线上设备与驱动的联动
 ********************************************************************
 */
/* 判断传入的设备与驱动是否匹配 */
void device_bind_driver (struct device *dev)
{
    list_add_before(&dev->drv->device, &dev->drv_list);
}

/* 将设备绑定到所属驱动的管理链表上 */
int  driver_prode_device (struct device_driver *drv, struct device *dev)
{
    if ((drv->bus->match) &&
        (drv->bus->match(dev, drv) <= 0))
        return -1;
    
    /* 记录设备所挂载的驱动对象 */
    dev->drv = drv;

    /* 驱动是否有回调函数需要执行 */
    if (drv->probe)
    {
        if (0 > drv->probe(dev))
        {
            dev->drv = NULL;
            return -1;
        }
    }
    /* 将设备绑定到驱动的管理链表中 */
    device_bind_driver(dev);

    return 0;
}

/* 将设备与所属的驱动进行分离 */
void device_release_driver (struct device *dev)
{
    /*将设备从链表移除 */
    list_del_init(&dev->drv_list);

    /* 执行所属驱动用户层注册的 remove 回调 */
    if (dev->drv->remove)
        dev->drv->remove(dev);
    
    /*重置设备的属性 */
    dev->drv = NULL;
    dev->driver_data = NULL;
}


/*
 ********************************************************************
 *          总线提供的操作驱动的接口
 ********************************************************************
 */
/* 将设备与总线中的所有驱动进行匹配 */
void driver_attach (struct device_driver *drv)
{
    ListEntry_t *ptr;
    struct device *dev;
    struct bus_type *bus = drv->bus;

    if ((drv == NULL) || (bus->match == NULL))
        return;
    
    /*遍历总线上的所有设备 */
    list_for_each(ptr, &bus->device->list)
    {
        /* 获取设备对象的结构体首地址 */
        dev = container_of(ptr, struct device, bus_list);

        /*判断设备与当前驱动是否匹配 */
        if(0 > driver_prode_device(drv, dev))
        {
            kprintf("drvier_attach: %s procb of %s fail !\n",
                dev->kobj.name,drv->name);
            return;
        }
    }
}


/*
 ********************************************************************
 *          总线提供的操作设备的接口
 ********************************************************************
 */
/* 将驱动与总线中的所有设备进行匹配 */
int device_attach (struct device *dev)
{
    ListEntry_t *ptr;
    struct device_driver *drv;
    struct bus_type *bus = dev->bus;

    if (dev == NULL)
        return -1;
    if (dev->drv)
    {
        device_bind_driver(dev);
        return 1;
    }

    if (bus->match)
    {
        /*寻找总线中的所有驱动 */
    
        list_for_each(ptr, &bus->driver->list)
        {
            /* 获取驱动的结构体首地址 */
            drv = container_of(ptr, struct device_driver, kobj.entry);

            /* 判断驱动与当前设备是否匹配 */
            if(0 > driver_prode_device(drv, dev))
            {
                kprintf("device_attach,%s procb of %s fail !\n",
                    dev->kobj.name, drv->name);
                return -1;
            }
        }
    }
    return 0;
}



/* 初始化总线子系统 */
int init_bus (void)
{
    return 0;
}
