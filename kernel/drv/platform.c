/*
 * 模拟 Linux 中的平台模块，管理平台的设备与驱动的变动
 * ( 目前未添加硬件资源的解析功能 )
 */
#include "platform.h"


/*
 *****************************************************
 *         平台管理模块的内部接口
 *****************************************************
 */
/* 当平台驱动中有设备被添加时，该接口被调用 */
static int platform_drv_probe (struct device *dev)
{
    struct platform_device *pdev;
    struct platform_driver *pdrv;

    pdev = container_of(dev, struct platform_device, dev);
    pdrv = container_of(dev->drv, struct platform_driver, drv);

    /* 调用用户注册到平台驱动上的 probe 函数 */
    if (pdrv->probe)
        pdrv->probe(pdev);

    return 0;
}

/* 当平台驱动中有设备被移除时，该接口被调用 */
static int platform_drv_remove (struct device *dev)
{
    struct platform_device *pdev;
    struct platform_driver *pdrv;

    pdev = container_of(dev, struct platform_device, dev);
    pdrv = container_of(dev->drv, struct platform_driver, drv);

    /* 调用用户注册到平台驱动上的 remove 函数 */
    if (pdrv->remove)
        pdrv->remove(pdev);

    return 0;
}

/* 当平台设备被释放时，该接口被调用 */
static void platform_device_release (struct device *dev)
{
    struct platform_device *pdev;

    pdev = container_of(dev, struct platform_device, dev);
    kfree(pdev);
}

/* 处理平台设备与驱动之间的匹配 */
static int platform_match (struct device *dev, struct device_driver *drv)
{
    struct platform_device *pdev;

    if ((dev == NULL) || (drv == NULL))
        return -1;

    pdev = container_of(dev, struct platform_device, dev);

    /* 对比平台设备与平台驱动的名字是否匹配 */
    return (kstrcpy(pdev->name, drv->name) == 0);
}


/*
 *****************************************************
 *         实例化的内部结构体成员
 *****************************************************
 */
/* 平台根设备 */
struct device platform_bus = 
{
    .name = "platform",
};

/* 平台总线 */
struct bus_type platform_bus_type = 
{
    .name = "platform",
    .match = platform_match,
};


 /*
 *****************************************************
 *         平台管理模块对外的接口
 *****************************************************
 */
/* 初始化整个平台模块 */
int init_platform_bus (void)
{
    device_register(&platform_bus);
    return bus_register(&platform_bus_type);
}



/* 平台设备的注册 */
int  platform_device_register (struct platform_device *pdev)
{
    if (pdev == NULL)
        return -1;

    /* 初始化平台设备的设备属性 */
    device_init(&pdev->dev);

    /* 将平台设备添加到操作系统内核 */
    return platform_device_add(pdev);
}

/* 平台设备的注销 */
void platform_device_unregister (struct platform_device *pdev)
{
    platform_device_del(pdev);
    platform_device_put(pdev);
}

/* 平台设备的添加 */
int  platform_device_add (struct platform_device *pdev)
{
    if (pdev == NULL)
        return -1;

    /* 设置平台设备对象默认的信息 */
    if (pdev->dev.parent == NULL)
        pdev->dev.parent = &platform_bus;
    pdev->dev.bus = &platform_bus_type;

    /* 设置平台设备对象注册到内核的名字 */
    kobject_setname(&pdev->dev.kobj, pdev->name);

    /* 将平台设备添加到操作系统内核 */
    return device_add(&pdev->dev);
}

/* 平台设备的删除 */
void platform_device_del (struct platform_device *pdev)
{
    if (pdev == NULL)
        return;

    /* 将平台设备从操作系统内核删除 */
    device_del(&pdev->dev);
}

/* 平台设备的释放 */
void platform_device_put (struct platform_device *pdev)
{
    if (pdev)
        put_device(&pdev->dev);
}

/* 平台设备的动态申请 */
struct platform_device *platform_device_alloc (char *name)
{
    struct platform_device *pdev = NULL;

    if (name == NULL)
        return NULL;

    /* 申请平台设备对象所占用的内存资源 */
    pdev = (struct platform_device *)kalloc(sizeof(*pdev));
    if (pdev == NULL)
        return NULL;

    /* 初始化平台设备的属性 */
    pdev->name = name;
    device_init(&pdev->dev);
    pdev->dev.release = platform_device_release;

    return pdev;
}



/* 平台驱动的注册 */
int platform_driver_register (struct platform_driver *pdrv)
{
    if (pdrv == NULL)
        return -1;

    /* 初始化平台驱动的属性 */
    pdrv->drv.bus = &platform_bus_type;
    pdrv->drv.probe = platform_drv_probe;
    pdrv->drv.remove = platform_drv_remove;

    /* 将平台驱动注册到操作系统内核 */
    return driver_register(&pdrv->drv);
}

/* 平台驱动的注销 */
void platform_driver_unregister (struct platform_driver *pdrv)
{
    /* 将平台驱动从操作系统内核注销 */
    if (pdrv)
        driver_unregister(&pdrv->drv);
}
