#ifndef __LIBS_LIST_H__
#define __LIBS_LIST_H__

#ifndef __ASSEMBLER__

#include "types.h"

/* *
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when manipulating
 * whole lists rather than single entries, as sometimes we already know
 * the next/prev entries and we can generate better code by using them
 * directly rather than using the generic single-entry routines.
 * */

typedef struct list_entry {
    struct list_entry *prev, *next;
}ListEntry_t;

static inline void list_init(ListEntry_t *elm) __attribute__((always_inline));
static inline void list_add(ListEntry_t *listelm, ListEntry_t *elm) __attribute__((always_inline));
static inline void list_add_before(ListEntry_t *listelm, ListEntry_t *elm) __attribute__((always_inline));
static inline void list_add_after(ListEntry_t *listelm, ListEntry_t *elm) __attribute__((always_inline));
static inline void list_del(ListEntry_t *listelm) __attribute__((always_inline));
static inline void list_del_init(ListEntry_t *listelm) __attribute__((always_inline));
static inline bool list_empty(ListEntry_t *list) __attribute__((always_inline));
static inline ListEntry_t *list_next(ListEntry_t *listelm) __attribute__((always_inline));
static inline ListEntry_t *list_prev(ListEntry_t *listelm) __attribute__((always_inline));

static inline void __list_add(ListEntry_t *elm, ListEntry_t *prev, ListEntry_t *next) __attribute__((always_inline));
static inline void __list_del(ListEntry_t *prev, ListEntry_t *next) __attribute__((always_inline));

/* 定义一个自动初始化的链表对象 */
#define LIST_INIT_OBJ(elm) \
    ListEntry_t elm={&elm, &elm}

/**
 * list_container_of - return the start address of struct type
 * ptr: known pointer address.
 * type: The type of structure that needs to be obtained.
 * member: The name of list_entry in the structure member.
 */
#define list_container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

/**
 * rt_list_for_each - iterate over a list
 * @param pos the rt_list_t * to use as a loop cursor.
 * @param head the head for your list.
 */
#define list_for_each(ptr, obj) \
    for (ptr = (obj)->next; ptr != (obj); ptr = ptr->next)

/**
 * rt_list_for_each_safe - iterate over a list safe against removal of list entry
 * @param pos the rt_list_t * to use as a loop cursor.
 * @param n another rt_list_t * to use as temporary storage
 * @param head the head for your list.
 */
#define list_for_each_safe(pos, n, head) \
    for (pos = (head)->next, n = pos->next; pos != (head); \
        pos = n, n = pos->next)

/* *
 * list_init - initialize a new entry
 * @elm:        new entry to be initialized
 * */
static inline void
list_init(ListEntry_t *elm) {
    elm->prev = elm->next = elm;
}

/* *
 * list_add - add a new entry
 * @listelm:    list head to add after
 * @elm:        new entry to be added
 *
 * Insert the new element @elm *after* the element @listelm which
 * is already in the list.
 * */
static inline void
list_add(ListEntry_t *listelm, ListEntry_t *elm) {
    list_add_after(listelm, elm);
}

/* *
 * list_add_before - add a new entry
 * @listelm:    list head to add before
 * @elm:        new entry to be added
 *
 * Insert the new element @elm *before* the element @listelm which
 * is already in the list.
 * */
static inline void
list_add_before(ListEntry_t *listelm, ListEntry_t *elm) {
    __list_add(elm, listelm->prev, listelm);
}

/* *
 * list_add_after - add a new entry
 * @listelm:    list head to add after
 * @elm:        new entry to be added
 *
 * Insert the new element @elm *after* the element @listelm which
 * is already in the list.
 * */
static inline void
list_add_after(ListEntry_t *listelm, ListEntry_t *elm) {
    __list_add(elm, listelm, listelm->next);
}

/* *
 * list_del - deletes entry from list
 * @listelm:    the element to delete from the list
 *
 * Note: list_empty() on @listelm does not return true after this, the entry is
 * in an undefined state.
 * */
static inline void
list_del(ListEntry_t *listelm) {
    __list_del(listelm->prev, listelm->next);
}

/* *
 * list_del_init - deletes entry from list and reinitialize it.
 * @listelm:    the element to delete from the list.
 *
 * Note: list_empty() on @listelm returns true after this.
 * */
static inline void
list_del_init(ListEntry_t *listelm) {
    list_del(listelm);
    list_init(listelm);
}

/* *
 * list_empty - tests whether a list is empty
 * @list:       the list to test.
 * */
static inline bool
list_empty(ListEntry_t *list) {
    return list->next == list;
}

/* *
 * list_next - get the next entry
 * @listelm:    the list head
 **/
static inline ListEntry_t *
list_next(ListEntry_t *listelm) {
    return listelm->next;
}

/* *
 * list_prev - get the previous entry
 * @listelm:    the list head
 **/
static inline ListEntry_t *
list_prev(ListEntry_t *listelm) {
    return listelm->prev;
}

/* *
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 * */
static inline void
__list_add(ListEntry_t *elm, ListEntry_t *prev, ListEntry_t *next) {
    prev->next = next->prev = elm;
    elm->next = next;
    elm->prev = prev;
}

/* *
 * Delete a list entry by making the prev/next entries point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 * */
static inline void
__list_del(ListEntry_t *prev, ListEntry_t *next) {
    prev->next = next;
    next->prev = prev;
}

#endif /* !__ASSEMBLER__ */

#endif /* !__LIBS_LIST_H__ */

