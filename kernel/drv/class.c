/*
 * 内核 class 管理模块
 * 主要提供内核设备分类管理的功能
 * (参考了linux2.6.11.12 和 linux5.10.120 的 class 源码)
 */
#include "defs.h"
#include "list.h"
#include "kobject.h"
#include "bus.h"
#include "class.h"



void class_release (struct kobject *kobj);


/**************** 内部变量 ***************/
/* class 模块共用的属性 */
static struct kobj_type class_ktype = 
{
    .release = class_release,
};
/* class 模块共用的子系统对象 */
static struct kset *class_kset;


/**************** 对内接口 ***************/
/* 当 class 对象引用计数为零时，该接口被系统内核所调用 */
void class_release (struct kobject *kobj)
{
    struct class *cls = NULL;

    if (kobj == NULL)
        return;
    /* 获取 class 对象的结构体首地址 */
    cls = container_of(kobj, struct class, subsys.kobj);
    if (cls == NULL)
        return;
    /* 释放 class 成员占用的内存空间 */
    kfree(cls);
}

/* 获取类对象 */
struct class *class_get (struct class *cls)
{
    if (cls)
        kset_get(&cls->subsys);
    return cls;
}

/* 释放类对象 */
void class_put (struct class *cls)
{
    if (cls)
        kset_put(&cls->subsys);
}


/**************** 对外接口 ***************/
/* 动态创建 class 对象 */
struct class *class_create (char *name)
{
    int retval;
    struct class *cls;

    cls = kalloc(sizeof(struct class));
    if (cls == NULL)
        return NULL;

    /* 初始化 class 对象的内容 */
    cls->name = name;

    /* 将 class 对象注册到系统内核 */
    retval = class_register(cls);
    if (retval < 0)
        goto err_class_create;

    return cls;
 err_class_create:
    kfree(cls);
    return NULL;
}

/* 注册 class 对象 */
int class_register (struct class *cls)
{
    if (cls == NULL)
        return -1;

    /* 初始化 class 对象的属性 */
    kset_init(&cls->subsys);
    list_init(&cls->klist_devices);
    list_init(&cls->interface);

    /* 设置 class 对象在系统内核里的名字 */
    kobject_setname(&cls->subsys.kobj, "%s", cls->name);

    /* 设置 class 对象关于当前模块的信息 */
    cls->subsys.kobj.kset  = class_kset;
    cls->subsys.kobj.ktype = &class_ktype;

    /* 将 class 对象注册到系统内核 */
    kset_register(&cls->subsys);

    return 0;
}

/* 卸载 class 对象 */
void class_unregister (struct class *cls)
{
    if (cls == NULL)
        return;
    kset_unregister(&cls->subsys);
}

/* 将设备对象添加到所属 class 的管理链表中 
 * ( 由 device_add 函数默认调用 )
 */
int class_device_add (struct device *dev)
{
    ListEntry_t *ptr = NULL;
    struct class_interface *class_intf = NULL;

    if ((dev == NULL) || (dev->class == NULL))
        return -1;
    
    /* 将设备挂载到 class 的管理链表中 */
    list_add_after(&dev->class_list, &dev->class->klist_devices);

    /* 遍历 class 上注册的所有接口对象 */
    list_for_each(ptr, &dev->class->interface)
    {
        class_intf = container_of(ptr, struct class_interface, node);
        if (class_intf->add_dev)
            class_intf->add_dev(dev, class_intf);
    }

    return 0;
}

/* 将设备对象从所属 class 的管理链表中移除 
 * ( 由 device_del 函数默认调用 )
 */
void class_device_del (struct device *dev)
{
    ListEntry_t *ptr = NULL;
    struct class_interface *class_intf = NULL;

    if ((dev == NULL) || (dev->class == NULL))
        return;

    /* 遍历 class 上注册的所有接口对象 */
    list_for_each(ptr, &dev->class->interface)
    {
        class_intf = container_of(ptr, struct class_interface, node);
        if (class_intf->del_dev)
            class_intf->del_dev(dev, class_intf);
    }

    /* 将设备从所属 class 的管理链表中移除 */
    list_del_init(&dev->class_list);
}

/* 注册类对象的操作接口 */
int class_interface_register (struct class_interface *class_intf)
{
    ListEntry_t *ptr = NULL;
    struct device *dev = NULL;
    struct class *parent = NULL;

    if ((class_intf == NULL) && (class_intf->class == NULL))
        return -1;
    
    /* 获取所属父 class */
    parent = class_get(class_intf->class);
    if (parent == NULL)
        return -1;

    /* 将类接口添加到所属类对象的管理接口 */
    list_add_after(&parent->interface, &class_intf->node);

    if (class_intf->add_dev)
    {
        /* 遍历 class 上的所有设备对象 */
        list_for_each(ptr, &parent->klist_devices)
        {
            dev = container_of(ptr, struct device, class_list);
            if (dev == NULL)
                continue;
            class_intf->add_dev(dev, class_intf);
        }
    }

    return 0;
}

/* 卸载类对象的操作接口 */
void class_interface_unregister (struct class_interface *class_intf)
{
    ListEntry_t *ptr = NULL;
    struct device *dev = NULL;

    if ((class_intf == NULL) && (class_intf->class == NULL))
        return;
    
    /* 将类接口从所属 class 的管理链表上移除 */
    list_del_init(&class_intf->node);

    if (class_intf->del_dev)
    {
        /* 遍历 class 上的所有设备对象 */
        list_for_each(ptr, &class_intf->class->klist_devices)
        {
            dev = container_of(ptr, struct device, class_list);
            if (dev == NULL)
                continue;
            class_intf->del_dev(dev, class_intf);
        }
    }
    
    /* 释放所属父 class */
    class_put(class_intf->class);
}



/* 初始化类对象模块 */
int init_class (void)
{
    class_kset = kset_create("class", NULL, NULL);
    if (class_kset == NULL)
        return -1;
    return 0;
}
