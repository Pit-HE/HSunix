/*
 * 管理实体文件系统的注册、挂载、卸载等操作
 * 1、注册的实体文件系统记录为文件系统对象: fsobj
 * 2、执行挂载后的实体文件系统为文件系统设备: fsdev
 */
#include "defs.h"
#include "file.h"
#include "fcntl.h"
#include "kstring.h"


/* 用于记录注册的文件系统信息 */
struct FsObject
{
    ListEntry_t          list;  /* 链接到 gFsObjectList */
    bool                 Multi; /* 文件系统实体是否允许挂载多次 */
    unsigned char        ref;   /* 挂载的次数 */
    struct FileSystem   *fs;    /* 记录传入的实体文件系统 */
};

/* 以链表的形式记录每个注册的文件系统 
 * ( 挂载的是 struct FsObject ) 
 */
LIST_INIT_OBJ(gFsObjectList);

/*   以树形图记录每个挂载的文件系统设备 
 * ( 记录的第一个是系统默认的根文件系统 )
 */
struct FsDevice *root_fsdev;


/********************************************************
 *      文件系统对象与文件系统设备模块内部专用接口
 *******************************************************/
/* 查找已注册的文件系统对象 */
static struct FsObject *find_fsobj (char *name)
{
    struct FsObject *fsobj = NULL;
    ListEntry_t *ptr = NULL;

    if (name == NULL)
        return NULL;
    
    list_for_each(ptr, &gFsObjectList)
    {
        fsobj = list_container_of(ptr, struct FsObject, list);
        if (0 == kstrcmp(fsobj->fs->name, name))
            return fsobj;
    }
    return NULL;
}

/* 申请文件系统设备所需的内存空间 */
static struct FsDevice *alloc_fsdev (char *path)
{
    struct FsDevice *fsdev;

    fsdev = (struct FsDevice *)kalloc(sizeof(struct FsDevice));
    if (fsdev == NULL)
        return NULL;
    
    /* 设置文件系统的挂载路径 */
    fsdev->path = (char *)kalloc(kstrlen(path) + 1);
    if (fsdev->path == NULL)
    {
        kfree(fsdev);
        return NULL;
    }

    kstrcpy(fsdev->path, path);

    list_init(&fsdev->siblist);
    list_init(&fsdev->sublist);

    return fsdev;
}

/* 释放文件系统设备结构体所占用的内存空间 */
static void free_fsdev (struct FsDevice *fsdev)
{
    if (fsdev == NULL)
        return;

    kfree(fsdev->path);
    kfree(fsdev);
}

/* 从树状图查找已挂载的文件系统设备 */
static struct FsDevice *find_fsdev (const char *path)
{
    struct FsDevice *fsdev = root_fsdev;
    struct FsDevice *nextdev = NULL;
    ListEntry_t *ptr = NULL;
    int path_len;

    if ((path == NULL) || (fsdev == NULL))
        return NULL;

    /* 确认找的不是根文件系统 */
    if (0 == kstrncmp(fsdev->path, path, kstrlen(path)))
        return fsdev;

    /* 确保子级链表不为空 */
    while(! list_empty(&fsdev->sublist))
    {
        /* 遍历子级链表 */
        list_for_each(ptr, &fsdev->sublist)
        {
            nextdev = list_container_of(ptr, struct FsDevice, siblist);
            path_len = kstrlen(nextdev->path);
            if (! kstrncmp(nextdev->path, path, path_len) &&
                (path[path_len] == '\0' || path[path_len] == '/'))
            {
                fsdev = nextdev;
                break;
            }
        }

        /*  */
        if (fsdev != nextdev)
            break;
    }
   
    return fsdev;
}

/* 将文件系统设备添加到管理树中 */
static int insert_fsdev (struct FsDevice *parent, struct FsDevice *child)
{
    if (child == NULL)
        return -1;
    
    if (parent == NULL)
    {
        /* 不存在父节点，则设置为系统默认的文件系统 */
        if (root_fsdev == NULL)
        {
            root_fsdev = child;
            root_fsdev->parent = NULL;
        }
    }
    else
    {
        /* 将文件系统设备添加到父设备之后，建立树状图 */
        child->parent = parent;
        kDISABLE_INTERRUPT();
        list_add_before(&parent->sublist, &child->siblist);
        kENABLE_INTERRUPT();
    }
    
    return 0;
}

/* 将文件系统设备从管理树中移除 */
static int remove_fsdev (struct FsDevice *fsdev)
{
    /* 该文件系统设备是否还有挂载有子设备 */
    if (list_empty(&fsdev->sublist))
    {
        list_del_init(&fsdev->siblist);
    }
    else
        return -1;

    return 0;
}


/********************************************************
 *      文件系统对象与文件系统设备模块对外的接口
 *******************************************************/
/* 将文件系统注册到内核中 ( 由实体文件系统初始化时调用 ) */
int fsobj_register (char *name, struct FileOperation *fops,
        struct FileSystemOps *fsops, uint multi)
{
    struct FsObject *fsobj;
    struct FileSystem *fs;

    fsobj = (struct FsObject *)kalloc(sizeof
                (struct FsObject));
    if (fsobj == NULL)
        return -1;
    fs = (struct FileSystem *)kalloc(sizeof
                (struct FileSystem));
    if (fs == NULL)
    {
        kfree(fsobj);
        return -1;
    }

    /* 初始化文件系统对象的信息 */
    fs->fops  = fops;
    fs->fsops = fsops;
    kstrcpy(fs->name, name);

    /* 初始化文件系统设备的结构体 */
    fsobj->fs = fs;
    fsobj->ref = 0;
    fsobj->Multi = multi;
    list_init(&fsobj->list);

    /* 将文件系统设备添加到注册管理链表上 */
    kDISABLE_INTERRUPT();
    list_add_before(&gFsObjectList, &fsobj->list);
    kENABLE_INTERRUPT();

    return 0;
}

/* 将实体文件系统挂载到可用区域 (传入的必须是绝对路径) */
int fsdev_mount (char *fsname, char *path, uint flag, void *data)
{
    int ret = 0;
    char *ap_path = NULL;   /* 绝对路径 */
    char *rp_path = NULL;   /* 相对路径 */
    struct FsObject *fsobj = NULL;
    struct FsDevice *chi_fsdev = NULL;
    struct FsDevice *per_fsdev = NULL;

    /* 获取文件系统要挂载的绝对路径 */
    ap_path = path_parser(NULL, path);
    if (ap_path == NULL)
        return -1;

    /* 查找已注册的文件系统 */
    fsobj = find_fsobj(fsname);
    if (fsobj == NULL)
        return -1;

    /* 判断该设备是否允许多次挂载 */
    if ((fsobj->Multi == FALSE) &&
        (fsobj->ref != 0))
        return -1;

    /* 查找文件系统要挂载时的父对象 */
    per_fsdev = find_fsdev(ap_path);

    /* 不允许在同一路径上重复挂载文件系统 */
    if ((per_fsdev != NULL) &&
        kstrlen(per_fsdev->path) == kstrlen(ap_path))
        return -1;

    /* 创建新的文件系统设备 */
    chi_fsdev = alloc_fsdev(ap_path);
    if (chi_fsdev == NULL)
        return -1;

    /* 将文件系统对象链接到文件系统设备 */
    chi_fsdev->fs = fsobj->fs;
    fsobj->ref += 1;    

    /* 将新文件系统挂载到其父文件系统之下 */
    insert_fsdev(per_fsdev, chi_fsdev);

    /* 调用实体文件系统的挂载接口 */
    if (chi_fsdev->fs->fsops->mount != NULL)
    {
        ret = chi_fsdev->fs->fsops->mount(chi_fsdev, 
                flag, data);
    }

    /* 去除路径中文件系统的挂载路径 */
    rp_path = path_fsdev(chi_fsdev, ap_path);

    /* 创建根目录下的 '.' 与 '..' */
    file_defaultdir(chi_fsdev, rp_path, flag, S_IRWXU);

    kfree(rp_path);
    kfree(ap_path);

    return ret;
}

/* 将实体文件系统从可用区域移除 */
int fsdev_unmount (char *path)
{
    int ret = 0;
    char *ap_path = NULL;
    struct FsObject *fsobj = NULL;
    struct FsDevice *fsdev = NULL;

    /* 获取要操作的绝对路径 */
    ap_path = path_parser(NULL, path);
    if (ap_path == NULL)
        return -1;

    /* 寻找该绝对路径下所挂载的文件系统 */
    fsdev = find_fsdev(ap_path);
    if (fsdev == NULL)
        return -1;
    if (fsdev == root_fsdev)
        return -1;
    if (fsdev->ref != 0)
        return -1;

    fsobj = find_fsobj(fsdev->fs->name);
    if (fsobj == NULL)
        return -1;
    fsobj->ref -= 1;

    remove_fsdev(fsdev);

    /* 调用实体文件系统的挂载接口 */
    if (fsdev->fs->fsops->unmount != NULL)
    {
        ret = fsdev->fs->fsops->unmount(fsdev);
    }

    free_fsdev(fsdev);
    kfree(ap_path);

    return ret;
}

/* 获取实体文件系统的结构体 (传入的必须是绝对路径) */
struct FsDevice *fsdev_get (const char *path)
{
    struct FsDevice *fsdev;

    fsdev = find_fsdev(path);
    fsdev->ref += 1;

    return fsdev;
}

/* 释放已获取的实体文件系统 */
void fsdev_put (struct FsDevice *fsdev)
{
    if (fsdev == NULL)
        return;
    if (fsdev->ref == 0)
        return;

    fsdev->ref -= 1;
}

