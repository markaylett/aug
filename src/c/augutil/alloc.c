/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/alloc.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/lock.h"
#include <stdlib.h>

static struct aug_allocs free_ = AUG_HEAD_INITIALIZER(free_);
AUG_ALLOCATOR(allocate_, &free_, aug_alloc, 64)

static void
destroyallocs_(struct aug_allocs* allocs)
{
    if (!AUG_EMPTY(allocs)) {
        aug_lock();
        AUG_CONCAT(&free_, allocs);
        aug_unlock();
    }
}

static void
destroyalloc_(struct aug_alloc* alloc)
{
    aug_lock();
    AUG_INSERT_TAIL(&free_, alloc);
    aug_unlock();
}

static struct aug_alloc*
createalloc_(void)
{
    struct aug_alloc* alloc;

    aug_lock();
    alloc = allocate_();
    aug_unlock();

    return alloc->mem_;
}
