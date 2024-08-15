/*
 * 模拟 linux 的总线设备驱动模型的设备模块
 * 
 * 1、参考 Linux 的 2.6.11.12 和 5.10.120 的源码而实现
 * 2、实现设备模块最核心的设备对象的管理功能
 */
#include "defs.h"
#include "device.h"


static void device_create_release (struct device *dev)
{
    if (dev)
        kfree(dev);
}

static void device_release (struct kobject *kobj)
{
    struct device *dev = NULL;

    dev = container_of(kobj, struct device, kobj);
    if (dev->release)
        dev->release(dev);
}
struct kobj_type device_ktype = 
{
    .release = device_release,
};

/* 设备子系统 */
struct kset *device_subsys;

/* 初始化设备对象 */
void device_init (struct device *dev)
{
    dev->kobj.kset = device_subsys;

    kobject_init(&dev->kobj);

    list_init(&dev->bus_list);
    list_init(&dev->drv_list);
    list_init(&dev->child_list);
    list_init(&dev->broth_list);
}

/* 将设备对象添加到操作系统内核 */
int  device_add  (struct device *dev)
{
    struct device *parent = NULL;

    /* 获取设备 */
    dev = get_device(dev);
    if (dev == NULL)
        return -1;

    /* 获取设备所属的父设备对象 */
    parent = get_device(dev->parent);
    dev->kobj.parent = &parent->kobj;

    /* 将设备对象添加到所属的 kset */
    if (0 > kobject_add(&dev->kobj))
        goto err_kobject_add;

    /* 将设备对象添加到所属的总线 */
    if (0 > bus_add_device(dev))
        goto err_bus_add_device;

    /* 将设备对象添加到所属的父设备管理链表 */
    if (parent)
    {
        kPortDisableInterrupt();
        list_add_after(&dev->child_list, &parent->child_list);
        kPortEnableInterrupt();
    }

err_bus_add_device:
    kobject_del(&dev->kobj);
err_kobject_add:
    if (parent)
        put_device(parent);
    return 0;
}

/* 删除操作系统内的设备对象 */
void device_del  (struct device *dev)
{
    struct device *parent = NULL;

    if (dev == NULL)
        return;
    parent = dev->parent;

    /* 将设备对象从所属父对象管理链表中移除 */
    kPortDisableInterrupt();
    if (parent)
    {
        list_del_init(&dev->broth_list);
        /* 释放对父设备的占用 */
        put_device(parent);
    }
    kPortEnableInterrupt();

    /* 将设备对象从所属总线中移除 */
    bus_remove_device(dev);

    /* 将设备对象从所属 kset 中移除 */
    kobject_del(&dev->kobj);
}

/* 注册设备对象 */
int  device_register (struct device *dev)
{
    device_init(dev);
    return device_add(dev);
}

/* 注销设备对象 */
void  device_unregister (struct device *dev)
{
    device_del(dev);
    put_device(dev);
}

/* 释放设备对象 */
void put_device (struct device *dev)
{
    if (dev)
        kobject_put(&dev->kobj);
}

/* 获取设备对象 */ 
struct device *get_device (struct device *dev)
{
    if (dev)
        kobject_get(&dev->kobj);
    return dev;
}

/* 销毁动态创建的设备对象 */
void device_destroy (struct device *dev)
{
    if (dev)
    {
        put_device(dev);
        device_unregister(dev);
    }
}

/* 动态创建设备对象 */
struct device *device_create (struct device *parent,
                void *drvdata, char *name)
{
    struct device *dev = NULL;

    if (name == NULL)
        return NULL;

    /* 获取内存空间存放设备对象 */
    dev = (struct device *)kalloc(sizeof(*dev));
    if (dev == NULL)
        return NULL;

    /* 初始化设备对象的信息 */
    device_init(dev);

    /* 设置设备对象的属性 */
    dev->parent = parent;
    dev->driver_data = drvdata;
    dev->release = device_create_release;
    kobject_setname(&dev->kobj, name);

    return dev;
}

/* 从设备管理链表中查找指定的子设备 */
struct device *device_find_child (struct device *parent, void *data,
                int (*match)(struct device *dev, void *data))
{
    struct device *dev = NULL;
    ListEntry_t *ptr, *tmp;

    if ((parent == NULL) || (match == NULL))
        return NULL;
    
    /* 遍历父设备的设备管理链表 */
    list_for_each_safe(ptr, tmp, &parent->child_list)
    {
        /* 获取子设备的结构体首地址 */
        dev = container_of(ptr, struct device, broth_list);

        /* 检查构造函数是否符合要求 */
        if (match(dev, data))
            break;
    }

    return dev;
}

/* 从设备管理链表中查找指定名字的子设备 */
struct device *device_find_child_by_name (struct device *parent, 
                char *name)
{
    struct device *dev = NULL;
    ListEntry_t *ptr, *tmp;

    if ((parent == NULL) || (name == NULL))
        return NULL;
    
    /* 遍历父设备的设备管理链表 */
    list_for_each_safe(ptr, tmp, &parent->child_list)
    {
        /* 获取子设备的结构体首地址 */
        dev = container_of(ptr, struct device, broth_list);

        /* 检查设备名字是否为目标设备 */
        if (kstrcpy(dev->kobj.name, name) == 0)
        {
            dev = get_device(dev);
            break;
        }
    }

    return dev;
}

/* 遍历父设备管理链表中的所有子设备 */
int device_for_each_child (struct device *parent, void *data,
                int (*fn)(struct device *dev, void *data))
{
    int error;
    struct device *dev = NULL;
    ListEntry_t *ptr, *tmp;

    if ((parent == NULL) || (fn == NULL))
        return -1;
    
    /* 遍历父设备的设备管理链表 */
    list_for_each_safe(ptr, tmp, &parent->child_list)
    {
        /* 获取子设备的结构体首地址 */
        dev = container_of(ptr, struct device, broth_list);

        /* 检查构造函数是否符合要求 */
        error = fn(dev, data);
        if (error <= 0)
            break;
    }

    return error;
}



/* 初始化设备模块 */
int init_device (void)
{
    device_subsys = kset_create("device", NULL, NULL);
    if (device_subsys == NULL)
        return -1;

    return kset_register(device_subsys);
}
