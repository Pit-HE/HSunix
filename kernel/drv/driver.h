
#ifndef __DRIVER_H__
#define __DRIVER_H__


#include "bus.h"


int driver_register (struct device_driver *drv);
int driver_unregister (struct device_driver *drv);
struct device_driver *driver_find (char *name, struct bus_type *bus);
struct device *driver_find_device (struct device_driver *drv,
            struct device *start, void *data,
            int (*match)(struct device *, void *));
struct device *driver_find_each_device (struct device_driver *drv,
            struct device *start, void *data,
            int (*fn)(struct device *, void *));



#endif
