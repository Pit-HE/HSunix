
#ifndef __CLASS_H__
#define __CLASS_H__



struct class 
{
    char            *name;
    /* 所属的(kset)子系统 */
    struct kset     subsys;
    /* 记录挂载的子设备对象 */
    ListEntry_t     klist_devices;
    /* 记录挂载的操作接口 */
    ListEntry_t     interface;
    /* 由用户注册的回调函数，该类对象被释放时调用 */
    void (*class_release)(struct class *class);
    /* 由用户注册的回调函数，类管理的设备被释放时调用 */
    void (*dev_release)(struct device *dev);
};


struct class_interface
{
    /* 用于挂载到 class 管理链表的节点 */
    ListEntry_t     node;
    /* 所属的类对象 */
    struct class    *class;
    /* 类设备添加时的回调接口 */
    int (*add_dev) (struct device *, struct class_interface *);
    /* 类设备删除时的回调接口 */
    int (*del_dev) (struct device *, struct class_interface *);
};



struct class *class_create (char *name);
int  class_register (struct class *cls);
void class_unregister (struct class *cls);
int  class_device_add (struct device *dev);
void class_device_del (struct device *dev);
int  class_interface_register (struct class_interface *class_intf);
void class_interface_unregister (struct class_interface *class_intf);
int  init_class (void);

#endif
