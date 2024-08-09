
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
    return 0;
}

/* 内部接口：遍历指定总线上的所有设备对象 */
static int __bus_for_each_dev (struct bus_type *bus, void *data, 
                int (*fn)(struct device *, void *))
{
    return 0;
}

/* 释放驱动上管理的所有设备 */
static void driver_detach (struct device_driver *drv)
{

}


 /*
 ********************************************************************
 *          操作总线的接口
 ********************************************************************
 */
/* 将总线对象注册到总线子系统 */
int  bus_register (struct bus_type *bus)
{
    return 0;
}

/* 注销总线子系统内的总线对象 */
int  bus_unregister (struct bus_type *bus)
{
    return 0;
}

/* 将设备添加到其所属的总线 */
int  bus_add_device (struct device *dev)
{
    return 0;
}

/* 将驱动添加到其所属的总线 */
int  bus_add_driver (struct device_driver *drv)
{
    return 0;
}

/* 将指定设备从总线上移除 */
int  bus_remove_device (struct device *dev)
{
    return 0;
}

/* 将指定驱动从总线上移除 */
int  bus_remove_driver (struct device_driver *drv)
{
    return 0;
}

/* 释放总线 */
void put_bus (struct bus_type *bus)
{

}

/* 获取总线 */
struct bus_type *get_bus (struct bus_type *bus)
{
    return NULL;
}

/* 遍历指定总线上的所有驱动对象 */
int bus_for_each_drv (struct bus_type *bus, struct device_driver *start,
            void *data, int (*fn)(struct device_driver *, void *))
{
    return 0;
}

/* 遍历指定总线上的所有设备对象 */
int bus_for_each_dev (struct bus_type *bus, struct device *start,
            void *data, int (*fn)(struct device *, void *))
{
    return 0;
}


/*
 ********************************************************************
 *          总线上设备与驱动的联动
 ********************************************************************
 */
/* 将设备绑定到所属驱动的管理链表上 */
int  driver_prode_device (struct device_driver *drv, struct device *dev)
{
    return 0;
}

/* 判断传入的设备与驱动是否匹配 */
void device_bind_driver (struct device *dev)
{

}

/* 将设备与所属的驱动进行分离 */
void device_release_driver (struct device *dev)
{

}


/*
 ********************************************************************
 *          总线提供的操作驱动的接口
 ********************************************************************
 */
/* 将设备与总线中的所有驱动进行匹配 */
void driver_attach (truct device_driver *drv)
{

}


/*
 ********************************************************************
 *          总线提供的操作设备的接口
 ********************************************************************
 */
/* 将驱动与总线中的所有设备进行匹配 */
void device_attach (truct device *dev)
{

}





/* 初始化总线子系统 */
int init_bus (void)
{
    return 0;
}