/*
 * 处理关于实体文件系统相关的操作
 */
#include "defs.h"
#include "file.h"

/* 以链表的形式记录每个注册的文件系统 */
LIST_INIT_OBJ(gFsRegistList);
/* 以链表的形式记录每个挂载的文件系统 */
LIST_INIT_OBJ(gFsMountList);

static struct FsDevice *alloc_fsdev (void)
{
    struct FsDevice *fsdev;
    struct FileSystem *fs;

    fsdev = (struct FsDevice *)kalloc(sizeof(struct FsDevice));
    if (fsdev == NULL)
        return NULL;
    fs = (struct FileSystem *)kalloc(sizeof(struct FileSystem));
    if (fs == NULL)
    {
        kfree(fsdev);
        return NULL;
    }

    fsdev->fs = fs;
    list_init (&fsdev->list);

    return fsdev;
}
static void free_fsdev (struct FsDevice *fsdev)
{
    if (fsdev == NULL)
        return;

    kfree(fsdev->fs);
    kfree(fsdev);
}

/* 从链表中查找指定已注册的文件系统 */
static struct FsDevice *find_fsdev (ListEntry_t *list, char *name)
{
    struct FsDevice   *fsdev;
    ListEntry_t     *ptr;

    list_for_each(ptr, list)
    {
        fsdev = list_container_of(ptr, struct FsDevice, list);
        if (0 == kstrcmp(fsdev->name, name))
            break;
    }
    return fsdev;
}
/* 拷贝注册链表中的文件系统设备，创建新的文件系统设备用于挂载 */
static struct FsDevice *dup_fsdev (struct FsDevice *old)
{
    struct FsDevice *new;

    if (old == NULL)
        return NULL;

    new = alloc_fsdev();
    if (new == NULL)
        return NULL;

    /* 拷贝文件系统对象的内容 */
    kstrcpy(new->fs->name, old->fs->name);
    new->fs->fops  = old->fs->fops;
    new->fs->fsops = old->fs->fsops;

    return new;
}


/* 将文件系统注册到内核中 ( 由实体文件系统初始化时调用 ) */
int fsdev_register (char *name, struct FileOperation *fops,
        struct FileSystemOps *fsops, unsigned int multi)
{
    struct FsDevice *fsdev;
    struct FileSystem *fs;

    fsdev = (struct FsDevice *)kalloc(sizeof(struct FsDevice));
    if (fsdev == NULL)
        return -1;
    fs = (struct FileSystem *)kalloc(sizeof(struct FileSystem));
    if (fs == NULL)
    {
        kfree(fsdev);
        return -1;
    }

    /* 初始化文件系统对象的信息 */
    fs->fops  = fops;
    fs->fsops = fsops;
    kstrcpy(fs->name, name);

    /* 初始化文件系统设备的结构体 */
    fsdev->fs = fs;
    fsdev->ref = 0;
    fsdev->Multi = multi;
    list_init(&fsdev->list);
    kstrcpy(fsdev->name, name);

    /* 将文件系统设备添加到注册管理链表上 */
    kDISABLE_INTERRUPT();
    list_add_before(&gFsRegistList, &fsdev->list);
    kENABLE_INTERRUPT();

    return 0;
}

/* 将实体文件系统挂载到可用区域 */
int fsdev_mount (char *fsname, char *mount_name,
        unsigned int flag, void *data)
{
    int ret = 0;
    struct Inode *root;
    struct FsDevice *reg_dev, *new_dev;

    /* 避免重复挂载 */
    if (find_fsdev(&gFsMountList, mount_name))
        return -1;

    /* 查找已注册的文件系统 */
    reg_dev = find_fsdev(&gFsRegistList, fsname);
    if (reg_dev == NULL)
        return -1;

    /* 判断该设备是否允许多次挂载 */
    if ((reg_dev->Multi == FALSE) &&
        (reg_dev->ref != 0))
        return -1;

    /* 创建新的文件系统设备并拷贝注册链表中的设备信息 */
    new_dev = dup_fsdev(reg_dev);
    if (new_dev == NULL)
        return -1;

    /* 创建文件系统的根目录节点，将其链接到挂载的文件系统 */
    root = inode_alloc();
    if (root == NULL)
    {
        free_fsdev(new_dev);
        return -1;
    }
    inode_init(root, flag, new_dev->fs->fops, INODE_DIR);
    root->fs = new_dev->fs;
    /* 将根文件节点链接到文件系统 */
    new_dev->fs->root = root;

    reg_dev->ref += 1;
    kstrcpy(new_dev->name, mount_name);

    kDISABLE_INTERRUPT();
    list_add_before(&gFsMountList, &new_dev->list);
    kENABLE_INTERRUPT();

    /* 调用实体文件系统的挂载接口 */
    if (new_dev->fs->fsops->mount != NULL)
        ret = new_dev->fs->fsops->mount(new_dev->fs, flag, data);

    return ret;
}

/* 将实体文件系统从可用区域移除 */
int fsdev_unmount (char *name)
{
    int ret = 0;
    struct FsDevice *fs_dev, *reg_dev;

    fs_dev = find_fsdev(&gFsMountList, name);
    if (fs_dev == NULL)
        return -1;
    if (fs_dev->ref != 0)
        return -1;

    kDISABLE_INTERRUPT();
    list_del_init(&fs_dev->list);
    kENABLE_INTERRUPT();

    reg_dev = find_fsdev(&gFsRegistList, fs_dev->fs->name);
    reg_dev->ref -= 1;

    /* 调用实体文件系统的挂载接口 */
    if (fs_dev->fs->fsops->unmount != NULL)
        ret = fs_dev->fs->fsops->unmount(fs_dev->fs);

    inode_free(fs_dev->fs->root);
    free_fsdev(fs_dev);

    return ret;
}

/* 获取实体文件系统的结构体 */
struct FileSystem *fsdev_get (char *name)
{
    struct FsDevice *fsdev;

    fsdev = find_fsdev(&gFsMountList, name);
    fsdev->ref += 1;

    return fsdev->fs;
}

/* 释放已获取的实体文件系统 */
void fsdev_put (char *name)
{
    struct FsDevice *fsdev;

    fsdev = find_fsdev(&gFsMountList, name);
    fsdev->ref -= 1;
}
