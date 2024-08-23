
#ifndef __KDEV_H__
#define __KDEV_H__


#include "list.h"
#include "defs.h"


#define DEVICE_MAGIC    0xDE5A


/* 设备对象的结构体类型 */
struct Device
{
    uint                    magic;  /* 标记设备的对象的魔幻数 */
    ListEntry_t             list;   /* 用于挂载到 gDevList 的链表对象 */
    struct FileOperation   *opt;    /* 设备的操作接口 */
    char                   *name;   /* 设备的名称 */
    uint                    ref;    /* 设备的引用计数 */
};


void init_dev (void);
void dev_free (struct Device *dev);
void dev_register (struct Device *dev);
void dev_unregister (struct Device *dev);
void dev_put (struct Device *dev);
struct Device *dev_alloc (const char *name);
struct Device *dev_get (const char *name);


#endif
