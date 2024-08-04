/*
 *  管理内核引用计数功能
 */
#include "defs.h"
#include "kref.h"


void kref_init (struct kref *ref)
{
    ref->refcount = 1;
}
void kref_get (struct kref *ref)\
{
    ref->refcount += 1;
}
void kref_put (struct kref *ref, void (*release) (struct kref *kref))
{
    if (ref->refcount > 0)
    {
        ref->refcount -= 1;
        if (ref->refcount == 0)
        {
            release(ref);
        }
    }
}



