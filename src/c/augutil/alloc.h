/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_ALLOC_H
#define AUGUTIL_ALLOC_H

/**
 * @file augutil/alloc.h
 *
 * Slab allocators for small objects.
 */

#include "augutil/config.h"
#include "augutil/list.h"

struct aug_alloc32 {
    AUG_ENTRY(aug_alloc32);
    char mem_[32];
};

AUG_HEAD(aug_allocs32, aug_alloc32);

struct aug_alloc64 {
    AUG_ENTRY(aug_alloc64);
    char mem_[64];
};

AUG_HEAD(aug_allocs64, aug_alloc64);

AUGUTIL_API void
aug_destroyallocs32(struct aug_allocs32* allocs32);

AUGUTIL_API void
aug_destroyalloc32(struct aug_alloc32* alloc32);

AUGUTIL_API struct aug_alloc32*
aug_createalloc32(void);

AUGUTIL_API void
aug_destroyallocs64(struct aug_allocs64* allocs64);

AUGUTIL_API void
aug_destroyalloc64(struct aug_alloc64* alloc64);

AUGUTIL_API struct aug_alloc64*
aug_createalloc64(void);

AUGUTIL_API void
aug_free(void* ptr);

AUGUTIL_API void*
aug_alloc(size_t size);

#endif /* AUGUTIL_ALLOC_H */
