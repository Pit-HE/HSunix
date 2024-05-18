/*
 * 处理关于实体文件系统相关的操作
 */
#include "defs.h"


/* 管理注册到内核的每个实体文件系统 */
struct fs_dev
{
    ListEntry_t          list;  /* 链接到 gFsList */
    unsigned int         ref;   /* 引用计数 */
    struct FileSystem   *fs;    /* 记录传入的实体文件系统 */
};


/* 以链表的形式记录每个注册的文件系统 */
LIST_INIT_OBJ(gFsRegistList);
/* 以链表的形式记录每个挂载的文件系统 */
LIST_INIT_OBJ(gFsMountList);


/* 从链表中查找指定已注册的文件系统 */
static struct fs_dev *find_fsdev (ListEntry_t *list, char *name)
{
    struct fs_dev   *fsdev;
    ListEntry_t     *ptr;

    list_for_each(ptr, list)
    {
        fsdev = list_container_of(ptr, struct fs_dev, list);
        if (0 == kstrcmp(fsdev->fs->name, name))
            break;
    }
    return fsdev;
}



/* 将文件系统注册到内核中 */
int fsdev_register (struct FileSystem *fs)
{
    struct fs_dev *fsdev;

    fsdev = (struct fs_dev *)kalloc(sizeof(struct fs_dev));
    if (fsdev == NULL)
        return -1;
    kmemset(fsdev, 0, sizeof(struct fs_dev));

    fsdev->fs = fs;
    fsdev->ref = 0;
    list_init(&fsdev->list);

    kDISABLE_INTERRUPT();
    list_add_before(&gFsRegistList, &fsdev->list);
    kENABLE_INTERRUPT();

    return 0;
}

/* 将实体文件系统挂载到可用区域 */
int fsdev_mount (char *name, unsigned int flag, void *data)
{
    int ret = 0;
    struct fs_dev *fsdev;

    fsdev = find_fsdev(&gFsRegistList, name);
    if (fsdev == NULL)
        return -1;

    if (fsdev->fs->fsops->mount != NULL)
        ret = fsdev->fs->fsops->mount(fsdev->fs, flag, data);

    kDISABLE_INTERRUPT();
    list_del_init(&fsdev->list);
    list_add_before(&gFsMountList, &fsdev->list);
    kENABLE_INTERRUPT();

    return ret;
}

/* 将实体文件系统从可用区域移除 */
int fsdev_unmount (char *name)
{
    int ret = 0;
    struct fs_dev *fsdev;

    fsdev = find_fsdev(&gFsMountList, name);
    if (fsdev == NULL)
        return -1;
    if (fsdev->ref != 0)
        return -1;

    if (fsdev->fs->fsops->unmount != NULL)
        ret = fsdev->fs->fsops->unmount(fsdev->fs);

    kDISABLE_INTERRUPT();
    list_del_init(&fsdev->list);
    list_add_before(&gFsRegistList, &fsdev->list);
    kENABLE_INTERRUPT();

    return ret;
}

/* 获取实体文件系统的结构体 */
struct FileSystem *fsdev_get (char *name)
{
    struct fs_dev *fsdev;

    fsdev = find_fsdev(&gFsMountList, name);
    fsdev->ref += 1;

    return fsdev->fs;
}

/* 释放已获取的实体文件系统 */
void fsdev_put (char *name)
{
    struct fs_dev *fsdev;

    fsdev = find_fsdev(&gFsMountList, name);
    fsdev->ref -= 1;
}
