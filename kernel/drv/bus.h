
#ifndef __BUS_H__
#define __BUS_H__


#include"defs.h"
#include "list.h"
#include "kobject.h"


struct device;
struct device_driver;


/* 总线描述符 */
struct bus_type
{
    char            *name;
    /* 总线内嵌的子系统 */
    struct kset     subsys;
    /* 记录挂载在总线上的所有设备 */
    struct kset     *device;
    /* 记录挂载在总线上的所有驱动 */
    struct kset     *driver;
    /* 处理总线上的设备与驱动的匹配 */
    int (*match)(struct device *dev, struct device_driver *drv);
};


/* 设备描述符 */
struct device
{
    /* 用于添加到设备子系统 */
    struct kobject              kobj;
    /* 用于挂载到所属总线的管理链表 */
    ListEntry_t                 bus_list;
    /* 用于挂载到所属驱动的管理链表 */
    ListEntry_t                 drv_list;
    /* 记录子设备的管理链表 */
    ListEntry_t                 child_list;
    /* 用于挂载到父设备的 child_list 链表中 */
    ListEntry_t                 broth_list;
    /* 记录设备所属对象的信息 */
    struct device               *parent;
    struct bus_type             *bus;
    struct device_driver        *drv;
    /* 私有数据域 */
    void                        *driver_data;
    /* 提供给继承该结构体的子类，注册其资源释放的接口 */
    void (*release) (struct device *dev);
};

struct device_driver
{
    char                        *name;
    /* 继承的内核对象属性 */
    struct kobject              kobj;
    /* 所属的总线 */
    struct bus_type             *bus;
    /* 记录挂载到当前驱动的设备 */
    ListEntry_t                 device;
    /* 处理设备的匹配 */
    int (*probe) (struct device *dev);
    /* 处理设备的移除 */
    int (*remove) (struct device *dev);
};

/* 操作总线的接口 */
int  bus_register (struct bus_type *bus);
int  bus_unregister (struct bus_type *bus);
int  bus_add_device (struct device *dev);
int  bus_add_driver (struct device_driver *drv);
void bus_remove_device (struct device *dev);
void bus_remove_driver (struct device_driver *drv);
void put_bus (struct bus_type *bus);
struct bus_type *get_bus (struct bus_type *bus);
int bus_for_each_drv (struct bus_type *bus, struct device_driver *start,
            void *data, int (*fn)(struct device_driver *, void *));
int bus_for_each_dev (struct bus_type *bus, struct device *start,
            void *data, int (*fn)(struct device *, void *));

/* 同时操作设备与驱动的接口 */
void device_bind_driver (struct device *dev);
int  driver_prode_device (struct device_driver *drv, struct device *dev);
void device_release_driver (struct device *dev);

/* 操作驱动的接口 */
void driver_attach (struct device_driver *drv);

/* 操作设备的接口 */
int  device_attach (struct device *dev);

/* 模块初始化的总接口 */
int init_bus (void);


#endif
