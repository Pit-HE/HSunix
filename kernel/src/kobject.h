/*
 * 内核对象管理模块的对外信息
 */
#ifndef __K_OBJECT_H__
#define __K_OBJECT_H__

#include "defs.h"
#include "list.h"
#include "kref.h"


#define KOBJ_NAME_LEN   32

/* 内核对象 (基本数据类型) */
struct kobject
{
    /* 名字 */
    char                *name;
    /* 内核引用计数 */
    struct kref          kref;
    /* 指向所属的父对象 */
    struct kobject      *parent;
    /* 用于和其他 kobject 组成链表 */
    ListEntry_t          entry;
    /* 指向所属的 kset */
    struct kset         *kset;
    /* 指向所属的 ktype */
    struct kobj_type    *ktype;
};

/* 内核对象集 
 * 1、通过双向链表用于管理同类型的 kobject
 * 2、可以挂载到另一个 kset 之下产生层次结构
 * 3、kset 本身也是一个特殊的 kobject，继承了 kobject 的内容
 */
struct kset
{
    /* 记录挂载到 kset 的所有 kobject */
    ListEntry_t              list;
    /* 内嵌的 kobject 属性 */
    struct kobject           kobj;
    /* 用于发送 kset 内 kobject 变动的通知 */
    struct kset_uevent_ops  *uevent_opt;
};

/* kobject 的属性 
 * 1、记录 kobject 的属性操作集合
 * 2、多个 kobject 可以有同一个 ktype
 * 2、后续有待扩展
 */
struct kobj_type
{
    void (*release)(struct kobject *);
};

/*  */
struct kset_uevent_ops
{
	int (*filter)(struct kset *kset, struct kobject *kobj);
	char *(*name)(struct kset *kset, struct kobject *kobj);
	int (* uevent)(struct kset *kset, struct kobject *kobj,
		      char *buffer);
};

/* 内核对象可能发生的事件集 */
enum kobj_action 
{
	KOBJ_ADD	= 0x01,	/* add event, for hotplug */
	KOBJ_REMOVE	= 0x02,	/* remove event, for hotplug */
	KOBJ_CHANGE	= 0x03,	/* a sysfs attribute file has changed */
	KOBJ_MOUNT	= 0x04,	/* mount event for block devices */
	KOBJ_UMOUNT	= 0x05,	/* umount event for block devices */
	KOBJ_OFFLINE= 0x06,	/* offline event for hotplug devices */
	KOBJ_ONLINE	= 0x07,	/* online event for hotplug devices */
};





void kobject_put (struct kobject *obj);
void kobject_init (struct kobject *kobj, struct kobj_type *ktype);
void kobject_cleanup (struct kobject *obj);
void kobject_release (struct kref *kref);
int  kobject_setname (struct kobject *obj, const char *fmt, ...);
int  kobject_rename (struct kobject *obj, char *new_name);
char *kobject_getname (struct kobject *obj);
int  kobject_add (struct kobject *obj);
void kobject_del (struct kobject *obj);
int  kobject_register (struct kobject *obj);
void kobject_unregister (struct kobject *obj);
struct kobject *kobject_get (struct kobject *obj);
struct kset *kset_create (char *name,
                struct kset_uevent_ops *uevent_ops,
                struct kobject *parent_kobj);

struct kobject *kset_find_kobj(struct kset *k, char *name);
struct kset *kset_get (struct kset *k);
void kset_put (struct kset *k);
void kset_init (struct kset *k);
int  kset_register (struct kset *k);
void kset_unregister (struct kset *k);


#endif
