/*
 * 提供虚拟文件系统处理文件路径字符串的功能接口
 */
#include "defs.h"
#include "file.h"
#include "fcntl.h"



/* 解析路径中第一个节点的名字, 并返回剩下的路径内容
 *
 * path: 要解析的路径
 * name：存放第一个节点字符的缓冲区
 */
char *path_getfirst (char *path, char *name)
{
    char *ptr = path;

    if (path == NULL)
        return NULL;

    /* 跳过文件路径上的斜杠 */
    while(*ptr == '/')
        ptr++;

    if (name != NULL)
    {
        /* 获取第一个节点的字符串 */
        while(*ptr != '/' && *ptr)
            *name++ = *ptr++;
        /* 标记字符串的结束 */
        *name = '\0';
    }
    else
    {
        while(*ptr != '/' && *ptr)
            ptr++;
    }

    /* 每次 ptr 都会停在文件路径的斜杠上 */
    return ptr;
}

/* 解析路径中最后一个节点的名字，并返回该节点前的父节点路径
 *
 * path: 要解析的路径
 * parentPath: 存放父节点路径的缓冲区
 * name：最后一个节点的名字
 *
 * 返回值：-1为失败
 */
int path_getlast (char *path, char *parentPath, char *name)
{
    char *p_path = NULL;

    if ((path == NULL) || (parentPath == NULL) || (name == NULL))
        return -1;

    /* 获取最后一个'/'的位置 */
    p_path = kstrrchr(path, '/');

    if (p_path == path)
    {
        parentPath[0] = '/';
        parentPath[1] = '\0';
    }
    else
    {
        kmemcpy(parentPath, path, p_path - path);
        parentPath[p_path - path] = '\0';
    }

    /* 拷贝时跳过'/' */
    kstrcpy(name, p_path + 1);
    return 0;
}

/* 格式化传入的文件路径，处理其中的 '.' 与 ".." */
char *path_formater (char *path)
{
    char *dst0 = NULL;
    char *dst = NULL;
    char *src = NULL;

    src = path;
    dst = path;

    dst0 = dst;
    while (1)
    {
        char c = *src;

        if (c == '.')
        {
            if (!src[1])
                src++; /* '.' and ends */
            else if (src[1] == '/')
            {
                /* './' case */
                src += 2;

                while ((*src == '/') && (*src != '\0'))
                    src++;
                continue;
            }
            else if (src[1] == '.')
            {
                if (!src[2])
                {
                    /* '..' and ends case */
                    src += 2;
                    goto up_one;
                }
                else if (src[2] == '/')
                {
                    /* '../' case */
                    src += 3;

                    while ((*src == '/') && (*src != '\0'))
                        src++;
                    goto up_one;
                }
            }
        }

        /* copy up the next '/' and erase all '/' */
        while ((c = *src++) != '\0' && c != '/')
            *dst++ = c;

        if (c == '/')
        {
            *dst++ = '/';
            while (c == '/')
                c = *src++;

            src--;
        }
        else if (!c)
            break;

        continue;

    up_one:
        dst--;
        if (dst < dst0)
        {
            kfree(path);
            return NULL;
        }
        while (dst0 < dst && dst[-1] != '/')
            dst--;
    }

    *dst = '\0';

    /* remove '/' in the end of path if exist */
    dst--;
    if ((dst != path) && (*dst == '/'))
        *dst = '\0';

    /* final check fullpath is not empty, for the special path of lwext "/.." */
    if ('\0' == path[0])
    {
        path[0] = '/';
        path[1] = '\0';
    }

    return path;
}

/* 解析传入的路径信息，将其转化为绝对路径作为返回值
 * 1、directory == NULL & filepath 为相对路径时，使用当前工作路径组成绝对路径
 * 2、directory == NULL & filepath 为绝对路径时，直接使用 filepath 作为绝对路径
 * 3、directory != NULL & filepath 为相对路径时，将两者组合称为绝对路径
 * 4、directory != NULL & filepath 为绝对路径时，直接使用 filepath 作为绝对路径
 */
char *path_parser (char *directory, char *filepath)
{
    char *path = NULL;

    if (filepath == NULL)
        return NULL;
    /* 使用当前进程的工作路径 */
    if (directory == NULL)
        directory = getProcCB()->cwd;
    /* 不允许直接操作相对路径 */
    if ((directory == NULL) && (filepath[0] != '/'))
        return NULL;

    if (filepath[0] != '/')
    {
        path = (char *)kalloc(kstrlen(directory) + kstrlen(filepath) + 2);
        if (path == NULL)
            return NULL;

        kstrcpy(path, directory);
        path[kstrlen(directory)] = '/';
        kstrcat(path, filepath);
    }
    else
    {
        path = kstrdup(filepath);
        if (path == NULL)
            return NULL;
    }

    /* 处理文件路径中的 '.' 与 '..' */
    return path_formater(path);
}

/* 设置进程的当前工作路径
 * ( 传入的必须是绝对路径 )
 */
int path_setcwd (char *path)
{
    ProcCB *pcb = NULL;
    char *cwd = NULL, *old_cwd = NULL;

    if ((path == NULL) || (path[0] != '/'))
        return -1;
    pcb = getProcCB();

    /* 格式化该路径 */
    path_formater(path);

    cwd = (char *)kalloc(kstrlen(path) + 1);
    if (cwd == NULL)
        return -1;
    kstrcpy(cwd, path);

    kDISABLE_INTERRUPT();
    old_cwd = pcb->cwd;
    pcb->cwd = cwd;
    kENABLE_INTERRUPT();

    kfree(old_cwd);

    return 0;
}

/* 获取进程的当前工作路径 */
char *path_getcwd (void)
{
    char *cwd = NULL;
    ProcCB *pcb = getProcCB();

    cwd = (char *)kalloc(kstrlen(pcb->cwd) + 1);
    if (cwd != NULL)
        kstrcpy(cwd, pcb->cwd);

    return cwd;
}
