
#include "defs.h"
#include "kstring.h"
#include "slab.h"


#define SLAB_ZONE_MAGIC     0x51ab51ab
#define SLAB_ZONE_LIMIT     10*1024     // 大小页的分界线

/* 标记 slab 中 zone 的大小区间 */
#define SLAB_ZONE_MIN_SIZE  16*1024
#define SLAB_ZONE_MAX_SIZE  32*1024

/* 标记 slab 中虚拟内存页的类型 */
#define SLAB_TYPE_FREE      0x00
#define SLAB_TYPE_SMALL     0x01
#define SLAB_TYPE_LARGE     0x02

#define SLAB_PAGE_SIZE      4096    // 单个物理内存页的大小
#define SLAB_ZONE_NUM       72      // slab 中划分的 zone 数量
#define SLAB_RELEASE_THRESH 2       // slab 中最大的空闲 zone 数量

/* 内存地址对齐 */
#define ALIGN_SIZE          4
#define ALIGN_UP(size, align)   (((size) + (align) -1) & ~((align) -1))
#define ALIGN_DOWN(size, align) ((size) & ~((align) - 1))

/* 获取物理内存页对应的控制块 */
#define get_memusage(slab, addr)    \
    (&(slab->memusage[((unsigned long)(addr) - slab->heap_start)/SLAB_PAGE_SIZE]))



/* chunk 头信息结构体 */
struct slab_chunk
{
    struct slab_chunk *next;
};

/* zone 控制块 */
struct slab_zone
{
    unsigned int        magic;
    /* 单向链表 */
    struct slab_zone    *next;
    /* 空闲 chunk 的数量 */
    unsigned int        free;
    /* 所有 chunk 的数量 */
    unsigned int        max;
    /* 指向第一个 chunk 的地址 */
    unsigned char       *baseptr;
    /* 记录 zone 未分配数组中第一个 chunk 的索引 */
    unsigned int        chunkidx;
    /* 记录当前 zone 中 chunk 的大小单位 */
    unsigned int        chunksize;
    /* 记录当前 zone 在 slab 的zone管理数组的小标索引 */ 
    unsigned int        index;
    /* 链表形式，记录被释放后的空闲 chunk*/
    struct slab_chunk   *freechunk;
};

/* 物理内存页信息记录结构体 */
struct slab_memusage
{
    uint32  type : 2;
    /*
     * 1、内存页类型为大页：记录当前物理内存页的数量
     * 2、内存页类型为小页：记录当前内存页在 zone 的第几页
     */
    uint32  size : 30;
};

/* 虚拟内存页的页头 */
struct slab_page
{
    struct slab_page    *next;
    /* 占用的物理内存页数量 */
    unsigned int        page;
    /* 填充 */
    char dummy[SLAB_PAGE_SIZE - sizeof(struct page *) - sizeof(int)];
};

/* slab 算法控制块 */
struct slab
{
    /* 记录总内存块可用区间的起始与结束地址 */
    unsigned long			heap_start;
    unsigned long			heap_end;
    /* 记录 slab 中所有可用物理内存页的信息 */
    struct slab_memusage	*memusage;
    /* 指针数据，记录所有类型的 zone */
    struct slab_zone		*zone_array[SLAB_ZONE_NUM];
    /* 记录完全空闲的 zone */
    struct slab_zone		*zone_free_list;
    /* 记录完全空闲 zone 的数量 */
    unsigned int			zone_free_cnt;
    /* 记录当前 slab 中，单个 zone 占用的内存大小 */
    unsigned int			zone_size;
    /* 记录当前 slab 中，单个 zone 拥有的物理内存页数量 */
    unsigned int			zone_page_cnt;
    /* 记录划分 slab 中，内存块为大页和小页的分界线 */
    unsigned int			zone_limit;
    /* 以链表形式记录每个物理内存页 */
    struct slab_page		*page_list;
};


/* 将传入的内存大小转化为二的次方幂
 * 
 * 返回值：该内存大小在 zone 数组中的索引号 
 */
unsigned int get_index (unsigned long *bytes)
{
    unsigned long n = (unsigned long)(*bytes);

    if (n < 128)
    {
        *bytes = n = (n + 7) & ~7;
        return (n / 8) - 1;
    }
    if (n < 256)
    {
        *bytes = n = (n + 15) & ~15;
        return (n / 16) + 17;
    }
    if (n < 8192)
    {
        if (n < 512)
        {
            *bytes = n = (n + 31) & ~31;
            return (n / 32) + 15;
        }
        if (n < 1024)
        {
            *bytes = n = (n + 63) & ~63;
            return (n / 64) + 23;
        }
        if (n < 2048)
        {
            *bytes = n = (n + 127) & ~127;
            return (n / 128) + 31;
        }
        if (n < 4096)
        {
            *bytes = n = (n + 255) & ~255;
            return (n / 256) + 39;
        }
        *bytes = n = (n + 511) & ~511;
        return (n / 512) + 47;
    }
    if (n < 16384)
    {
        *bytes = n = (n + 1023) & ~1023;
        return (n / 1024) + 55;
    }
    return 0;
}

/* 从 slab 中申请指定数量的物理内存页 */
void *slab_page_alloc (struct slab *slab, unsigned int npages)
{
    struct slab_page *tmp = NULL; 
	struct slab_page *qtr = NULL;
	struct slab_page **prev = NULL;

    if ((slab == NULL) || (npages == 0))
        return NULL;

    for (prev = &slab->page_list; (qtr = *prev) != NULL; prev = &(qtr->next))
	{
		/* 物理内存块需要拆分 */
		if (qtr->page > npages)
		{
			tmp = qtr + npages;
			tmp->next = qtr->next;
			tmp->page = qtr->page - npages;
			*prev = tmp;
			break;
		}
		/* 物理内存块大小刚刚好 */
		if (qtr->page == npages)
		{
			*prev = qtr->next;
			break;
		}
	}

    return qtr;
}

/* 将内存页释放回 slab 对象中 */
void slab_page_free (struct slab *slab, void *addr, unsigned int npages)
{
	struct slab_page *obj = NULL;
	struct slab_page *qtr = NULL;
	struct slab_page **prev = NULL;

	if (((unsigned long)addr % SLAB_PAGE_SIZE) != 0)
		return;
	if ((slab == NULL) || (addr == NULL))
		return;
	
	obj = (struct slab_page*)addr;

	for (prev = &slab->page_list; (qtr = *prev) != NULL; prev = &(qtr->next))
	{
		/* 寻找与目标内存前一块相邻的内存块 */
		if ((qtr + qtr->page) == obj)
		{
			qtr->page += npages;
			/* 判断是否与后一块内存相邻 */
			if ((qtr + qtr->page) == qtr->next)
			{
				qtr->page += qtr->next->page;
				qtr->next  = qtr->next->next;
			}
			return;
		}
		/* 寻找与目标内存后一块相邻的内存块 */
		if ((obj + npages) == qtr)
		{
			obj->page = npages + qtr->page;
			obj->next = qtr->next;
			*prev = obj;
			return;
		}
		if (qtr > (obj + npages))
				break;
	}
	obj->page = npages;
	obj->next = qtr;
	*prev = obj;
}

/* 将物理内存块添加到 slab 的 page_list 成员中 */
void slab_page_init (struct slab *slab, void *addr, unsigned npages)
{
	if ((slab==NULL) || (addr==NULL))
		return;
	slab->page_list = NULL;
	slab_page_free(slab, addr, npages);
}

struct slab *slab_init (void *begin_addr, unsigned long size)
{
	struct slab *slab;
	unsigned long limsize, npages, temp;
	unsigned long start_addr, begin_align, end_align;

	/* 格式化该内存块的头 */
	slab = (struct slab*)ALIGN_UP((unsigned long)begin_addr, ALIGN_SIZE);

	/* 计算去除头信息后可用内存块的起始地址 */
	start_addr = (unsigned long)slab + sizeof(struct slab);

	/* 计算对齐后该内存块可用的起始地址与结束地址 */
	begin_align = (unsigned long)ALIGN_UP((unsigned long)start_addr, SLAB_PAGE_SIZE);
	end_align = (unsigned long)ALIGN_DOWN((unsigned long)start_addr + size, SLAB_PAGE_SIZE);
	if (begin_align > end_align)
		return NULL;
	
	/* 计算该可用内存块的信息 */
	limsize = end_align - begin_align;
	npages = limsize / SLAB_PAGE_SIZE;

	/* 清空 slab 控制块，并设置初始化信息 */
	kmemset(slab, 0, sizeof(struct slab));

	slab->heap_start = begin_align;
	slab->heap_end   = end_align;

	/* 将整个物理内存块切分为物理内存页大小，并以链表形式进行链接 */
	slab_page_init (slab, (void*)begin_align, npages);

	/* 设置单个 zone 的大小 */
	slab->zone_size = SLAB_ZONE_MIN_SIZE;

	slab->zone_limit = SLAB_ZONE_LIMIT;
	slab->zone_page_cnt = slab->zone_size / SLAB_PAGE_SIZE;

	temp = npages * sizeof(struct slab_memusage);

	/* 确保申请到的物理内存是以页为单位大小 */
	temp = ALIGN_UP(temp, SLAB_PAGE_SIZE);
	slab->memusage = slab_page_alloc(slab, temp / SLAB_PAGE_SIZE);

	return slab;
}

void *slab_alloc (struct slab *slab, unsigned long size)
{
	unsigned long npages, index;
	struct slab_zone *zone = NULL;
	struct slab_chunk *chunk = NULL;
	struct slab_memusage *mcb = NULL;

	if ((slab == NULL) || (size == 0))
		return NULL;
	
	/*
	 * 处理大内存块分配的情况
	 */
	if (size >= slab->zone_limit)
	{
		/* 页对齐，确保申请的内存大小以物理页大小为单位 */
		size = ALIGN_UP(size, SLAB_PAGE_SIZE);
		npages = size / SLAB_PAGE_SIZE;
		chunk = slab_page_alloc(slab, npages);
		if (chunk == NULL)
			return NULL;

		/* 设置 slab 中记录物理内存页信息的控制块 */
		mcb = get_memusage(slab, chunk);
		mcb->type = SLAB_TYPE_LARGE;
		mcb->size = npages;

		return chunk;
	}

	/*
	 * 处理小内存块分配的情况
	 */
	index = get_index(&size);
	if (index >= SLAB_ZONE_NUM)
		return NULL;
	
	/* 处理 zone 不为空的情况 */
	if (slab->zone_array[index] != NULL)
	{
		zone = slab->zone_array[index];

		/* 判断 zone 中的 chunk 是否已经使用完 */
		if (--zone->free == 0)
		{
			slab->zone_array[index] = zone->next;
			zone->next = NULL;
		}
		
		if ((zone->chunkidx + 1) != zone->max)
		{
			/* 正常分配 */
			zone->chunkidx += 1;
			chunk = (struct slab_chunk*)(zone->baseptr + zone->chunkidx * size);
		}
		else
		{
			/* 已无未分配的 chunk，则从已释放的空闲链表中获取 */
			if (zone->freechunk == NULL)
				chunk = NULL;
			chunk = zone->freechunk;
			zone->freechunk = zone->freechunk->next;
		}
	}
	/* 处理 zone 为空的情况，获取新的 zone 并初始化 */
	else
	{
		unsigned int off;

		/* 判断 slab 中是否有被释放当还未回收的 zone 对象 */
		if (slab->zone_free_list != NULL)
		{
			zone = slab->zone_free_list;
			slab->zone_free_list = zone->next;
			slab->zone_free_cnt -= 1;
		}
		else
		{
			/* 获取新的内存页作为 zone */
			zone = slab_page_alloc (slab, slab->zone_size / SLAB_PAGE_SIZE);
			if (zone == NULL)
				return NULL;

			mcb = get_memusage(slab, zone);
			/* 更新已分配的这些物理内存页在 slab 中的控制块信息 */
			for (off = 0; off < slab->zone_page_cnt; off++)
			{
				mcb->type = SLAB_TYPE_SMALL;
				mcb->size = off;
				mcb++;
			}
		}

		/* 格式化该新 zone，并分配第一个 chunk 给用户 */
		kmemset (zone, 0, sizeof(struct slab_zone));

		/* 判断 zone 的头信息占用多少个 chunk */
		off = sizeof(struct slab_zone) / size;
		if (off == 0) off = 1;
		off += ((sizeof(struct slab_zone) % size) != 0) ? 1:0;

		/* 初始化 zone 的头信息 */
		zone->magic = SLAB_ZONE_MAGIC;
		zone->max   = slab->zone_size/size - off;
		zone->free  = zone->max - 1;
		zone->baseptr = (unsigned char*)(zone + off);
		zone->chunkidx = 0;
		zone->chunksize = size;
		zone->index = index;

		/* 划分出给用户的 chunk */
		chunk = (struct slab_chunk*)(zone->baseptr);

		/* 将 zone 添加到 slab 对象的指针数组中 */
		zone->next = slab->zone_array[index];
		slab->zone_array[index] = zone;
	}
	
	return chunk;
}

void slab_free (struct slab *slab, void *ptr)
{
	struct slab_zone *zone = NULL;
	struct slab_chunk *chunk = NULL;
	struct slab_memusage *mcb = NULL;

	if ((slab == NULL) || (ptr == NULL))
		return;
	if ((unsigned long)ptr < (unsigned long)(slab->heap_start))
		return;
	if ((unsigned long)ptr > (unsigned long)(slab->heap_end))
		return;
	
	mcb = get_memusage(slab, ptr);
	/*
	 * 处理大内存页的释放
	 */
	if (mcb->type == SLAB_TYPE_LARGE)
	{
		slab_page_free (slab, ptr, mcb->size);
		mcb->size = 0;
		return;
	}
	/* 
	 * 处理小内存页的释放
	 */
	else
	{
		/* 获取该内存地址所属的 zone 和 chunk 信息 */
		zone = (struct slab_zone*)(((unsigned long)ptr & ~(SLAB_PAGE_SIZE-1)) - \
				mcb->size * SLAB_PAGE_SIZE);
		chunk = (struct slab_chunk*)ptr;
		
		/* 格式化 chunk 信息，并将其添加到 zone 中 */
		chunk->next = zone->freechunk;
		zone->freechunk = chunk;

		/*  更新 zone 的信息，判断是否需要添加到所属 slab 的管理数组中 */
		if (zone->free++ == 0)
		{
			zone->next = slab->zone_array[zone->index];
			slab->zone_array[zone->index] = zone;
		}
	}

	/* 处理整个 zone 对象都是空闲未使用的情况 */
	if ((zone->free == zone->max) && ((zone->next) || 
		(slab->zone_array[zone->index] != zone)))
	{
		struct slab_zone **pz;

		/* 找到 zone 所在链表的前一个 zone */
		for (pz = &slab->zone_array[zone->index]; *pz != zone;
			 pz = &((*pz)->next));

		/* 将当前 zone 从链表中移除 */
		*pz = zone->next;

		zone->magic = 0;

		/* 将 zone 添加到 slab 的空闲链表中 */
		zone->next = slab->zone_free_list;
		slab->zone_free_list = zone;

		slab->zone_free_cnt += 1;

		/* 处理 slab 空闲链表超过指定数量的情况，将多余 zone 释放 */
		if (slab->zone_free_cnt > SLAB_RELEASE_THRESH)
		{
			/* 将 zone 从 slab 的空闲链表内移除 */
			zone = slab->zone_free_list;
			slab->zone_free_list = zone->next;
			slab->zone_free_cnt -= 1;

			/* 重置该 zone 在 slab 中对应的物理内存页控制块 */
			unsigned int i;
			mcb = get_memusage(slab, zone);
			for (i=0; i<slab->zone_page_cnt; i++)
			{
				mcb->type = SLAB_TYPE_FREE;
				mcb->size = 0;
				mcb++;
			}

			/* 释放该 zone 占用的物理内存页 */
			slab_page_free (slab, zone, slab->zone_page_cnt);
		}
	}
}

void *slab_realloc (struct slab *slab, void *ptr, unsigned long size)
{
	void *nptr = NULL;
	struct slab_zone *zone = NULL;
	//struct slab_chunk *chunk = NULL;
	struct slab_memusage *mcb = NULL;

	if (slab == NULL)
		return NULL;
	if ((unsigned long)ptr < (unsigned long)(slab->heap_start))
		return NULL;
	if ((unsigned long)ptr > (unsigned long)(slab->heap_end))
		return NULL;

	if (ptr == NULL)
		return slab_alloc(slab, size);
	
	if (size == 0)
	{
		slab_free (slab, ptr);
		return NULL;
	}

	/* 获取物理地址所对应的内存页控制块 */
	mcb = get_memusage(slab, (void*)((unsigned long)ptr & ~(SLAB_PAGE_SIZE - 1)));

	/* 处理大内存块的情况 */
	if (mcb->type == SLAB_TYPE_LARGE)
	{
		unsigned long nsize;
		nsize = mcb->size * SLAB_PAGE_SIZE;

		/* 申请新的物理内存 */
		nptr = slab_alloc (slab, size);
		if (nptr == NULL)
			return NULL;

		/*  执行数据搬运和释放 */
		kmemcpy (nptr, ptr, (size > nsize) ? nsize : size);
		slab_free (slab, ptr);
	}
	/* 处理小内存块的情况 */
	else if (mcb->type == SLAB_TYPE_SMALL)
	{
		/* 获取内春地址所属的 zone */
		zone = (struct slab_zone*)(((unsigned long)ptr & ~(SLAB_PAGE_SIZE - 1)) - 
					(mcb->size * SLAB_PAGE_SIZE));
		
		/* 将传入的内存大小格式化为二的次方幂 */
		get_index(&size);

		/* 判断原先的 chunk 是否符合最新的大小要求 */
		if (zone->chunksize >= size)
			return ptr;

		/* 申请新的物理内存 */
		nptr = slab_alloc (slab, size);
		if (nptr == NULL)
			return NULL;

		kmemcpy(nptr, ptr, 
			(size > zone->chunksize) ? zone->chunksize:size);
		slab_free (slab, ptr);
	}

    return nptr;
}



/*
*****************************
*       封装的对外接口
*****************************
*/
// static struct slab *g_slab;

void init_slab (void)
{

}

// void *alloc_page (void)
// {
//     return slab_alloc(g_slab, PGSIZE);
// }

// void free_page (void *pa)
// {
//     slab_free(g_slab, pa);
// }

// void kfree (void *addr)
// {
//     slab_free(g_slab, addr);
// }

// void *kalloc (unsigned long size)
// {
//     return slab_alloc(g_slab, size);
// }












