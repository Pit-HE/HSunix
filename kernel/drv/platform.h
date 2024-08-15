
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "driver.h"
#include "device.h"


/* 平台设备描述符 */
struct platform_device
{
    char            *name;
    struct device   dev;
};


/* 平台驱动描述符 */
struct platform_driver
{
    struct device_driver   drv;

    /* 由用户注册的，驱动与设备匹配时的回调函数 */
    int (*probe)  (struct platform_device *);

    /* 由用户注册的，驱动与设备分离时的回调函数 */
    int (*remove) (struct platform_device *);
};


/* 初始化整个平台模块 */
int init_platform_bus (void);

/* 平台设备的操作接口 */
int  platform_device_register (struct platform_device *pdev);
void platform_device_unregister (struct platform_device *pdev);
int  platform_device_add (struct platform_device *pdev);
void platform_device_del (struct platform_device *pdev);
void platform_device_put (struct platform_device *pdev);
struct platform_device *platform_device_alloc (char *name);

/* 平台驱动的操作接口 */
int  platform_driver_register (struct platform_driver *pdrv);
void platform_driver_unregister (struct platform_driver *pdrv);


#endif
