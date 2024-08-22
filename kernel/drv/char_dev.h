
#ifndef __CHAR_DEV_H__
#define __CHAR_DEV_H__


#include "types.h"
#include "list.h"


struct cdev
{
    struct kobject          kobj;   /* 内嵌的内核对象属性 */
    struct FileOperation    *fops;  /* 字符设备操作接口 */
    ListEntry_t             list;   /* 链表节点 */
    dev_t                   dev;    /* 设备号 */
    unsigned int            count;  /* 设备号的范围 */
};


/* 设备号管理接口 */
int  register_chrdev_region (dev_t from, unsigned count, char *name);
int  alloc_chrdev_region (dev_t *dev, unsigned baseminor, unsigned count, char *name);
void unregister_chrdev_region (dev_t from, unsigned count);

/* 字符设备对外的总接口 */
void init_chrdev (void);
int  chrdev_open (struct File *filp);
int  register_chrdev (unsigned int major, char *name, struct FileOperation *fops);
void unregister_chrdev (unsigned int major, char *name);

/* 字符设备管理接口 */
void cdev_init (struct cdev *cdev, struct FileOperation *fops);
struct cdev *cdev_alloc (void);
int  cdev_add (struct cdev *p, dev_t dev, unsigned count);
void cdev_del (struct cdev *p);
struct kobject *cdev_get (struct cdev *p);
void cdev_put (struct cdev *p);
void cdev_set_parent (struct cdev *p, struct kobject *kobj);
int  cdev_device_add (struct cdev *cdev, struct device *dev);
void cdev_device_del (struct cdev *cdev, struct device *dev);

#endif
