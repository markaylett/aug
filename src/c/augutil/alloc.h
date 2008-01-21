/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_ALLOC_H
#define AUGUTIL_ALLOC_H

/**
 * @file augutil/alloc.h
 *
 * Small object allocator.
 */

#include "augutil/config.h"
#include "augutil/list.h"

struct aug_alloc {
    AUG_ENTRY(aug_alloc);
    char mem_[64];
};

AUG_HEAD(aug_allocs, aug_alloc);

#endif /* AUGUTIL_ALLOC_H */
