/*
 * 内核对象管理模块
 * 主要提供 linux 中 kobject 与 kset 的功能
 */
#include "kobject.h"
#include <stdarg.h>


/* 获取指定的内核对象 (可嵌套) */
struct kobject *kobject_get (struct kobject *obj)
{
    if (obj)
        kref_get(&obj->kref);
    return obj;
}

/* 释放指定的内核对象 (可嵌套) */
void kobject_put (struct kobject *obj)
{
    if (obj)
        kref_put(&obj->kref, kobject_release);
}

/* 初始化传入的内核对象 */
void kobject_init (struct kobject *obj)
{
    if (obj == NULL)
        return;
    kmemset(obj, 0, sizeof(struct kobject));

    kref_init(&obj->kref);
    list_init(&obj->entry);
    kset_get(obj->kset);
}

/* 清除内核对象占用的资源 
 * 1、释放内核对象名字占用的内存
 * 2、释放内核对象所属 kset 的引用计数
 * 3、释放内核对象 kset 的释放接口
 * 4、释放内核对象的父对象的引用计数
 */
void kobject_cleanup (struct kobject *obj)
{
    if (obj == NULL)
        return;

    if (obj->name)
        kfree(obj->name);
    obj->name = NULL;

    if (obj->kset)
        kset_put(obj->kset);
    if (obj->ktype &&
        obj->ktype->release)
        obj->ktype->release(obj);
    if (obj->parent)
        kobject_put(obj->parent);
}

/* 释放内核对象占用的资源
 * ( 主要用于给内核引用计数接口作为回调函数 )
 */
void kobject_release (struct kref *kref)
{
    kobject_cleanup(container_of(kref, struct kobject, kref));
}

/* 发送内核对象变动的 uevent 事件 */
void kobject_uevent (struct kobject *obj, enum kobj_action action)
{
    if (obj == NULL)
        return;
    switch(action)
    {
        case KOBJ_ADD:
            break;
        case KOBJ_REMOVE:
            break;
        case KOBJ_CHANGE:
            break;
        case KOBJ_MOUNT:
            break;
        case KOBJ_UMOUNT:
            break;
        case KOBJ_OFFLINE:
            break;
        case KOBJ_ONLINE:
            break;
        default:break;
    }
}

/* 设置内核对象的名字 */
int kobject_setname (struct kobject *obj, const char *fmt, ...)
{
    int need;
    char *name;
    va_list args;
    int len = KOBJ_NAME_LEN;

    if ((obj == NULL) || (fmt == NULL))
        return -1;
    
    /* 申请新的内存空间存放内核对象的名字 */
    name = (char *)kalloc(KOBJ_NAME_LEN);
    if (name == NULL)
        return -1;

    /* 解析不定长字符串的内容 */
    va_start(args,fmt);
	need = vsnprintf(name,len,fmt,args);
	va_end(args);

    /* 要写入的名字是否过大 */
    if (need > KOBJ_NAME_LEN)
    {
        kfree(name);
        return -1;
    }

    /* 释放内核设备名原先占用的内存空间 */
    if (obj->name != NULL)
        kfree(obj->name);
    obj->name = name;

    return 0;
}

/* 重新设置内核对象的名字 */
int kobject_rename (struct kobject *obj, char *new_name)
{
    int error;

    kobject_get(obj);
    error = kobject_setname(obj, new_name);
    kobject_put(obj);
    
    return error;
}

/* 获取内核对象的名字 */
char *kobject_getname (struct kobject *obj)
{
    return obj->name;
}

/* 将 kobject 添加到系统内核 
 * 1、该功能将被多次调用，所以抽象成独立函数
 * 2、将 kobject 添加到所属的 kset
 * 3、设置 kobject 所属的父节点
 */
int kobject_add (struct kobject *obj)
{
    int error = 0;
    struct kobject *parent;

    if (obj == NULL)
        return -1;
    
    /* 递增该节点的引用计数 */
    if ((obj = kobject_get(obj)) == NULL)
        return -1;
    
    /* 获取父节点 */
    parent = kobject_get(obj->parent);

    if (obj->kset)
    {
        kDISABLE_INTERRUPT();
        /* 若是未设置默认的父对象，则让其指向 kset */
        if (parent == NULL)
            parent = kobject_get(&obj->kset->kobj);
        /* 挂载到所属的内核对象集中 */
        list_add_after(&obj->kset->list, &obj->entry);
        kENABLE_INTERRUPT();
    }
    obj->parent = parent;
    /* 发送 uevent */
    kobject_uevent(obj, KOBJ_ADD);

    return error;
}

/* 将 kobject 从系统内核删除 */
void kobject_del (struct kobject *obj)
{
    if (obj)
    {
        kDISABLE_INTERRUPT();
        list_del_init(&obj->entry);
        kENABLE_INTERRUPT();
    }
    kobject_uevent(obj, KOBJ_REMOVE);
    kobject_put(obj);
}

/* 将 kobject 注册到系统内核 */
int kobject_register (struct kobject *obj)
{
    kobject_init(obj);
    return kobject_add(obj);
}

/* 将 kobject 从系统内核注销 */
void kobject_unregister (struct kobject *obj)
{
    kobject_del(obj);
    kobject_put(obj);
}



/* 获取内核对象集 */
struct kset *kset_get (struct kset *k)
{
    if (k)
        kobject_put(&k->kobj);
    return k;
}

/* 释放内核对象集 */
void kset_put (struct kset *k)
{
    if (k)
        kobject_put(&k->kobj);
}

/* 初始化内核对象集 */
void kset_init (struct kset *k)
{
    kobject_init(&k->kobj);
    list_init(&k->list);
}

/* 将内核对象集添加到系统内核
 * 1、将对象集的 kobj 属性添加到父 kset 的管理链表
 * 2、kset 主要用于管理 kobject 对象的链接与删除
 * 3、kset 允许挂载到其他 kset 之下
 */
int kset_add (struct kset *k)
{
    if (k == NULL)
        return -1;

    /* 设置对象集的父对象 */
    if (!k->kobj.parent && !k->kobj.kset && k->parent)
        k->kobj.parent = &k->parent->kobj;

    /* 将内核对象集的 kobject 属性添加到内核 */
    return kobject_add(&k->kobj);
}

/* 通过名字在内核对象集中查找指定的内核对象 */
struct kobject *kset_find_kobj(struct kset *k, char *name)
{
    ListEntry_t *ptr;
    struct kobject *kobj = NULL;

    list_for_each(ptr, &k->list)
    {
        kobj = container_of(ptr, struct kobject, entry);
        if (kstrcpy(kobj->name, name) == 0)
        {
            kobj = kobject_get(kobj);
            break;
        }
    }

    return kobj;
}

/* 注册内核对象集 */
int kset_register (struct kset *k)
{
    kset_init(k);
    return kset_add(k);
}

/* 注销已存在的内核对象集 */
void kset_unregister (struct kset *k)
{
    kobject_unregister(&k->kobj);
}
