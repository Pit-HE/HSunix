
#ifndef __DEVICE_H__
#define __DEVICE_H__


#include "list.h"
#include "defs.h"
#include "file.h"


#define DEVICE_MAGIC    0xDE5A


/* 设备对象的结构体类型 */
struct Device
{
    unsigned int            magic;  /* 标记设备的对象的魔幻数 */
    ListEntry_t             list;   /* 用于挂载到 gDevList 的链表对象 */
    struct FileOperation    opt;    /* 设备的操作接口 */
    char                   *name;   /* 设备的名称 */
    unsigned int            ref;    /* 设备的引用计数 */
};



#endif
