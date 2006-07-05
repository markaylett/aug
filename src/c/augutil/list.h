/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_LIST_H
#define AUGUTIL_LIST_H

#include "augsys/queue.h"

/** By convention, 'entries_' is used as the field name.  This somewhat
    simplifies the interface. */

#define AUG_HEAD(name, type) \
STAILQ_HEAD(name, type)

#define AUG_HEAD_INITIALIZER(head) \
STAILQ_HEAD_INITIALIZER(head)

#define AUG_ENTRY(type) \
STAILQ_ENTRY(type) entries_

#define AUG_CONCAT(head1, head2) \
STAILQ_CONCAT(head1, head2)

#define AUG_EMPTY(head) \
STAILQ_EMPTY(head)

#define AUG_FIRST(head) \
STAILQ_FIRST(head)

#define AUG_FOREACH(var, head) \
STAILQ_FOREACH(var, head, entries_)

#define AUG_FOREACH_SAFE(var, head, tvar) \
STAILQ_FOREACH_SAFE(var, head, entries_, tvar)

#define AUG_INIT(head) \
STAILQ_INIT(head)

#define AUG_INSERT_AFTER(head, tqelm, elm) \
STAILQ_INSERT_AFTER(head, tqelm, elm, entries_)

#define AUG_INSERT_HEAD(head, elm) \
STAILQ_INSERT_HEAD(head, elm, entries_)

#define AUG_INSERT_TAIL(head, elm) \
STAILQ_INSERT_TAIL(head, elm, entries_)

#define AUG_LAST(head, type) \
STAILQ_LAST(head, type, entries_)

#define AUG_NEXT(elm) \
STAILQ_NEXT(elm, entries_)

#define AUG_REMOVE(head, elm, type) \
STAILQ_REMOVE(head, elm, type, entries_)

#define AUG_REMOVE_HEAD(head) \
STAILQ_REMOVE_HEAD(head, entries_)

#define AUG_REMOVE_HEAD_UNTIL(head, elm) \
STAILQ_REMOVE_HEAD_UNTIL(head, elm, entries_)

#define AUG_ALLOCATOR(name, head, type, bsize) \
static struct type* \
name(void) \
{ \
    struct type* it; \
    if (AUG_EMPTY((head))) { \
        struct type* end; \
        if (!(it = (struct type*)malloc(sizeof(struct type) * (bsize)))) { \
            aug_setposixerrinfo(__FILE__, __LINE__, ENOMEM); \
            return NULL; \
        } \
        for (end = it + (bsize); it < end; ++it) \
            AUG_INSERT_TAIL((head), it); \
    } \
    it = AUG_FIRST((head)); \
    AUG_REMOVE_HEAD((head)); \
    return it; \
}

#define	AUG_REMOVE_PREVPTR(var, varp, head) \
do { \
    if (!AUG_NEXT((var))) \
        (head)->stqh_last = (varp); \
    *(varp) = AUG_NEXT((var)); \
} while (0)

#endif /* AUGUTIL_LIST_H */
