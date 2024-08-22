/*
 * 字符设备管理模块
 */
#include "kobject.h"
#include "device.h"
#include "file.h"
#include "defs.h"
#include "kobj_map.h"
#include "char_dev.h"


/* 字符设备模块共用的内核对象哈希散列管理表 */
static struct kobj_map *cdev_map;
#define MAX_PROBE_HASH  255

/* 字符设备的设备号描述符 */
static struct char_device_struct
{
    /* 单向链表节点 */
    struct char_device_struct   *next;
    /* 主设备号 */
    unsigned int                major;
    /* 次设备号的起始值 */
    unsigned int                baseminor;
    /* 次设备号的大小范围 */
    int                         minorct;
    /* 名字 */
    char                        *name;
    /* 所管理的字符设备对象 */
    struct cdev                 *cdev;

} *chardevs[MAX_PROBE_HASH];/* 管理设备号描述符的指针数组 */


/*  
 *************************************************
 *                 设备号管理接口
 *************************************************
 */
/* 将主设备号转换为哈希索引值 */
static inline int major_to_HashIndex (int major)
{
    return (major % MAX_PROBE_HASH);
}

/* 添加新的设备号管理对象到链表中 */
static struct char_device_struct *__register_chrdev_region(unsigned int major, 
                unsigned int baseminor, int minorct, char *name)
{
    int i;
    struct char_device_struct *cd, *curr, *prev = NULL;

    cd = (struct char_device_struct *)kalloc(sizeof(*cd));
    if (cd == NULL)
        return NULL;

    /* 自动分配主设备号 */
    if (major == 0)
    {
        for(i=0; i<MAX_PROBE_HASH; i++)
            if (chardevs[i] == NULL)
                break;

        if (i == MAX_PROBE_HASH)
            goto _error_cdev_region;

        major = i;
    }

    /* 获取主设备号对于的哈希索引值 */
    i = major_to_HashIndex(major);

    /* 遍历单向链表，查找设备号管理对象的插入位置 */
    for (curr = chardevs[i]; curr; prev = curr, curr=curr->next)
    {
        if (curr->major < major)
            continue;
        if (curr->major > major)
            break;
        if (curr->baseminor + curr->minorct <= baseminor)
            continue;
        if (curr->baseminor > baseminor)
            break;

        /* 若无可以插入的地方，则执行错误处理 */
        goto _error_cdev_region;
    }

    /* 初始化该设备号管理对象 */
    cd->next = NULL;
    cd->major = major;
    cd->baseminor = baseminor;
    cd->minorct = minorct;
    cd->name = name;
    cd->cdev = NULL;

    /* 将设备号管理对象加入单向链表 */
    if (prev == NULL)
    {
        cd->next = curr;
        chardevs[i] = cd;
    }
    else
    {
        cd->next = prev->next;
        prev->next = cd;
    }

    return cd;
 _error_cdev_region:
    kfree(cd);
    return NULL;
}

/* 移除设备号管理链表中的指定对象 */
static struct char_device_struct *__unregister_chrdev_region(
                unsigned int major, unsigned int baseminor, int minorct)
{
    int i;
    struct char_device_struct *pd, *prev = NULL;

    /* 获取主设备号对于的哈希索引值 */
    i = major_to_HashIndex(major);

    /* 遍历单向链表，查找指定的设备号管理对象 */
    for(pd = chardevs[i]; pd; prev = pd, pd = pd->next)
    {
        if ((pd->major == major) && 
            (pd->baseminor == baseminor) &&
            (pd->minorct == minorct))
            break;
    }

    /* 是否找到了所需的目标 */
    if (pd != NULL)
    {
        if (prev == NULL)
            chardevs[i] = NULL;
        else
            prev->next = pd->next;
    }

    return pd;
}

/* 对外接口：注册新的静态设备号管理对象 */
int register_chrdev_region (dev_t from, unsigned count, char *name)
{
    struct char_device_struct *cd;
    dev_t max = from + count;
    dev_t n, next;

    /* 处理注册的设备数量多于主设备号下可用次设备号数量的情况 */
    for (n=from; n < max; n = next)
    {
        next = MKDEV(MAJOR(n)+1, 0);
        if (next > max)
            next = max;

        cd = __register_chrdev_region(MAJOR(n),MINOR(n), next-n, name);
        if (cd == NULL)
            goto _error_chrdev_region;
    }

    return 0;
 _error_chrdev_region:
    for (max = n, n=from; n < max; n = next)
    {
        next = MKDEV(MAJOR(n)+1, 0);
        if (next > max)
            next = max;
        cd = __unregister_chrdev_region(MAJOR(n), MINOR(n), next - n);
        kfree(cd);
    }
    return -1;
}

/* 对外接口：注册新的动态设备号管理对象 */
int alloc_chrdev_region (dev_t *dev, unsigned baseminor, unsigned count, char *name)
{
    struct char_device_struct *cd;

    cd = __register_chrdev_region(0, baseminor, count, name);
    if (cd == NULL)
        return -1;

    *dev = MKDEV(cd->major, cd->baseminor);
    return 0;
}

/* 对外接口：注销已有的设备号管理对象 */
void unregister_chrdev_region (dev_t from, unsigned count)
{
    dev_t n, next, max = from + count;

    for (n=from; n < max; n = next)
    {
        next = MKDEV(MAJOR(n)+1, 0);
        if (next > max)
            next = max;
       kfree(__unregister_chrdev_region(MAJOR(n),MINOR(n), next-n));
    }
}



/*  
 *************************************************
 *                 字符设备管理接口
 *************************************************
 */
static struct kobject *cdev_probe(dev_t dev, int *part, void *data)
{
    return NULL;
}
/* 初始化整个字符设备模块 */
void init_chrdev (void)
{
    cdev_map = kobj_map_init(cdev_probe);
}



/*
 * 
 *  总接口：字符设备对外的总接口
 *
 */
/* 用于注册到新创建的字符设备所属的 inode 中 */
struct FileOperation def_cdev_fops = 
{
    .open = chrdev_open,
};
/* 
 * 字符设备统一的 open 操作接口
 *
 * 1、默认传入的 inode 中已经包含有字符设备的信息 
 * 2、通过该接口找到真正要操作的字符设备，并替换对应的操作接口
 */
int chrdev_open (struct File *filp)
{
    int index, ret;
    struct Inode *inode;
    struct kobject *kobj;
    struct cdev *cd, *new;

    if (filp == NULL)
        return -1;
    inode = filp->inode;

    /* 确认 inode 上是否记录有完整的字符设备对象的信息 */
    cd = inode->i_cdev;
    if (cd == NULL)
    {
        /* 通过设备号去查找该字符设备对象 */
        kobj = kobj_lookup(cdev_map, inode->i_rdev, &index);
        if (kobj == NULL)
            return -1;

        /* 获取该字符设备对象的结构体首地址 */
        new = container_of(kobj, struct cdev, kobj);
        
        /* 再次检查 inode 中是否有字符设备对象的信息 */
        cd = inode->i_cdev;
        if (cd == NULL)
        {
            /* 将字符设备对象的信息写入 inode 中 */
            inode->i_cdev = cd = new;
            list_add_after(&inode->i_devices, &new->list);
            new = NULL;
        }
        else if (cdev_get(cd) == NULL)
        {
            return -1;
        }
    }
    else if (cdev_get(cd) == NULL)
    {
        return -1;
    }

    /* 处理 kobj_lookup 对 new 内核对象的递增 */
    cdev_put(new);

    /* 将 inode 和 file 中对文件的操作替换为字符设备的操作接口 */
    inode->fops = cd->fops;
    filp->fops  = cd->fops;

    /* 执行操作接口中的 open 功能 */
    if (filp->fops->open)
    {
        ret = filp->fops->open(filp);
        if (ret <= 0)
        {
            cdev_put(cd);
            return -1;
        }
    }

    return 0;
}

/* 注册字符设备
 *
 * major：主设备号
 * name：字符设备的名字
 * fops：字符设备操作接口
 *
 * 返回值：字符设备号
 */
int register_chrdev (unsigned int major, char *name, struct FileOperation *fops)
{
    struct cdev *cdev;
    struct char_device_struct *cd;

    /* 注册字符设备号 */
    cd = __register_chrdev_region(major, 0, 256, name);
    if (cd == NULL)
        return -1;

    /* 申请字符设备对象 */
    cdev = cdev_alloc();
    if (cdev == NULL)
        goto error_cdev_alloc;

    /* 将字符设备对象添加到系统内核 */
    if (0 > cdev_add(cdev, MKDEV(cd->major, 0), 256))
        goto error_cdev_add;

    /* 建立设备号管理对象与字符设备的链接 */
    cd->cdev = cdev;

    /* 返回主设备号 */
    return major ? 0:cd->major;
 error_cdev_add:
    kobject_put(&cdev->kobj);
 error_cdev_alloc:
    kfree(__unregister_chrdev_region(major, 0, 256));
    return -1;
}

/* 注销字符设备 */
void unregister_chrdev (unsigned int major, char *name)
{
    struct char_device_struct *cd;

    /* 将设备号管理结构体从链表中移除 */
    cd = __unregister_chrdev_region(major, 0, 256);
    if (cd == NULL)
        return;

    /* 删除字符设备对象 */
    cdev_del(cd->cdev);
    /* 释放设备号管理结构体占用的内存空间 */
    kfree(cd);
}


/* 
 *
 *  处理字符设备从内核中移除时的释放操作
 *
 */
/* 处理静态申请字符设备的释放 */
void cdev_defualt_release (struct kobject *kobj)
{
    kobject_put(kobj->parent);
}
/* 处理动态申请字符设备的释放 */
void cdev_dynamic_release (struct kobject *kobj)
{
    kfree(container_of(kobj, struct cdev, kobj));
    kobject_put(kobj->parent);
}
static struct kobj_type ktype_cdev_default = 
{
    .release = cdev_defualt_release,
};
static struct kobj_type ktype_cdev_dynamic = 
{
    .release = cdev_dynamic_release,
};



/* 
 *
 *  字符设备模块对外的功能接口
 *
 */
/* 初始化一个字符设备对象 */
void cdev_init (struct cdev *cdev, struct FileOperation *fops)
{
    kmemset(cdev, 0, sizeof(*cdev));

    kobject_init(&cdev->kobj, &ktype_cdev_default);
    cdev->fops = fops;
    list_init(&cdev->list);
}

/* 动态申请一个字符设备对象 */
struct cdev *cdev_alloc (void)
{
    struct cdev *cd;

    cd = (struct cdev *)kalloc(sizeof(*cd));
    if (cd == NULL)
        return NULL;

    kobject_init(&cd->kobj, &ktype_cdev_dynamic);
    list_init(&cd->list);

    return cd;
}

/* 添加一个字符设备到系统内核 */
int cdev_add (struct cdev *p, dev_t dev, unsigned count)
{
    int retval;

    p->dev = dev;
    p->count = count;

    retval = kobj_map(cdev_map, dev, count, cdev_probe, NULL, p);
    if (retval < 0)
        return -1;

    kobject_get(p->kobj.parent);
    return 0;
}

/* 从系统内核移除一个字符设备 */
void cdev_del (struct cdev *p)
{
    kobj_unmap(cdev_map, p->dev, p->count);
    /* 
     * 当字符设备引用计数为 0 真正被释放时，会调用注册的
     * ktype_cdev_default 或 ktype_cdev_dynamic 中的接口
     */
    kobject_put(&p->kobj);
}

/* 获取字符设备对象 */
struct kobject *cdev_get (struct cdev *p)
{
    if (p == NULL)
        return NULL;

    return kobject_get(&p->kobj);
}

/* 释放字符设备对象 */
void cdev_put (struct cdev *p)
{
    if (p == NULL)
        return;

    return kobject_put(&p->kobj);
} 

/* 设置字符设备的父类 */
void cdev_set_parent (struct cdev *p, struct kobject *kobj)
{
    if ((p == NULL) || (kobj == NULL))
        return;
    p->kobj.parent = kobj;
}

/* 将字符设备以及其父设备添加到系统内核 */
int cdev_device_add (struct cdev *cdev, struct device *dev)
{
    if ((cdev == NULL) || (dev == NULL))
        return -1;

    cdev_set_parent(cdev, &dev->kobj);
    cdev_add(cdev, cdev->dev, 1);
    device_add(dev);
    return 0;
}

/* 将字符设备以及其父设备从系统内核移除 */
void cdev_device_del (struct cdev *cdev, struct device *dev)
{
    if ((cdev == NULL) || (dev == NULL))
        return;

    device_del(dev);
    cdev_del(cdev);
}
