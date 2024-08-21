
#ifndef __TYPES_H__
#define __TYPES_H__



#ifndef NULL
#define NULL        (void*)0
#endif

#ifndef TRUE
#define TRUE        1
#endif

#ifndef FALSE
#define FALSE       0
#endif


typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned char  uchar;


typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;
typedef unsigned long  uint64;


typedef unsigned char  bool;
typedef unsigned long  pde_t;
typedef unsigned long  size_t;


typedef unsigned int   dev_t;
#define MINORBITS	20      /* 次设备号占据的 bit 位数量 */
#define MINORMASK	((1U << MINORBITS) - 1)
#define MAJOR(dev)	((unsigned int) ((dev) >> MINORBITS))
#define MINOR(dev)	((unsigned int) ((dev) & MINORMASK))
#define MKDEV(ma,mi)	(((ma) << MINORBITS) | (mi))


#endif
