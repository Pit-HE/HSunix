
#ifndef __DEVICE_H__
#define __DEVICE_H__


#include "bus.h"


int init_device (void);


void device_init (struct device *dev);
int  device_add  (struct device *dev);
void device_del  (struct device *dev);
int  device_register (struct device *dev);
int  device_unregister (struct device *dev);
void put_device (struct device *dev);
struct device *get_device (struct device *dev);
void device_destroy (struct device *dev);
struct device *device_create (struct device *parent,
                void *drvdata, char *name);
struct device *device_find_child (struct device *parent, void *data,
                int (*match)(struct device *dev, void *data));
struct device *device_find_child_by_name (struct device *parent, 
                char *name);
int   device_for_each_child (struct device *parent, void *data,
                int (*fn)(struct device *dev, void *data));



#endif
