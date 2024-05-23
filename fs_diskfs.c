/*
 *      磁盘文件系统，该文件系统将会是内核默认的文件系统
 *
 * 1、通过调用内核设备管理模块中的磁盘设备进行数据交换
 * 2、模拟 xv6 文件系统对于块的划分来管理磁盘对象
 */
#include "defs.h"
#include "file.h"
#include "fs_diskfs.h"




