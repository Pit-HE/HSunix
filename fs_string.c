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
char *fstr_getfirst (char *path, char *name)
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
int fstr_getlast (char *path, char *parentPath, char *name)
{
 #if 0
    char *p_path, *q_path;

    if ((path == NULL) || (parentPath == NULL) || (name == NULL))
        return -1;
    p_path = q_path = path;

    // /* 跳过根目录的斜杠，以及处理只传入根目录的情况 */
    // while(*p_path == '/')
    //     p_path++;
    // if (*p_path == '\0')
    // {
    //     parentPath[0] = '/';
    //     parentPath[1] = '\0';
    //     return 0;
    // }

    // while(1)
    // {
    //     while(*p_path != '/' && *p_path)
    //         p_path++;

    //     if (*p_path != '\0')
    //     {
    //         p_path += 1;    /* 跳过斜杠 */
    //         q_path = p_path;
    //     }
    //     else
    //     {
    //         /* q_path 已停留在子文件名的开头
    //          * p_path 已停留在字符串的结尾处
    //          */
    //         if (q_path == path)
    //         {
    //             parentPath[0] = '/';
    //             parentPath[1] = '\0';
    //             q_path += 1;
    //         }
    //         else
    //         {
    //             kmemcpy(parentPath, path, q_path - path - 1);
    //             parentPath[q_path - path - 1] = '\0';
    //         }
    //         kmemcpy(name, q_path, p_path - q_path);
    //         name[p_path - q_path] = '\0';
    //         break;
    //     }
    // }
 #else
    char *p_path;

    if ((path == NULL) || (parentPath == NULL) || (name == NULL))
        return -1;

    p_path = kstrrchr(path, '/');

    if (p_path == path)
    {
        parentPath[0] = '/';
        parentPath[1] = '\0';
    }
    else
    {
        kmemcpy(parentPath, path, p_path - path - 1);
        parentPath[p_path - path - 1] = '\0';
    }
    kstrcpy(name, p_path + 1);
 #endif

    return 0;
}

/* 格式化传入的文件路径，处理其中的 '.' 与 ".." */
char *fstr_formater (char *directory, char *path)
{
    char *fullpath;
    char *dst0, *dst, *src;

    if (path[0] != '/') /* it's a absolute path, use it directly */
    {
        fullpath = (char *)kalloc(kstrlen(directory) + kstrlen(path) + 2);

        if (fullpath == NULL)
            return NULL;

        kstrcpy(fullpath, directory);
        fullpath[kstrlen(directory)] = '/';
        kstrcat(fullpath, path);
    }
    else
    {
        fullpath = kstrdup(path); /* copy string */

        if (fullpath == NULL)
            return NULL;
    }

    src = fullpath;
    dst = fullpath;

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
            kfree(fullpath);
            return NULL;
        }
        while (dst0 < dst && dst[-1] != '/')
            dst--;
    }

    *dst = '\0';

    /* remove '/' in the end of path if exist */
    dst--;
    if ((dst != fullpath) && (*dst == '/'))
        *dst = '\0';

    /* final check fullpath is not empty, for the special path of lwext "/.." */
    if ('\0' == fullpath[0])
    {
        fullpath[0] = '/';
        fullpath[1] = '\0';
    }

    return fullpath;
}


