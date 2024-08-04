/*
 * 记录内核引用计数模块对外的内容
 */
#ifndef __KREF_H__
#define __KREF_H__



struct kref
{
    unsigned int refcount;
};


void kref_init (struct kref *ref);
void kref_get  (struct kref *ref);
void kref_put  (struct kref *ref, void (*release) (struct kref *kref));



#endif
