/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_MMAP_H
#define AUGSYS_MMAP_H

/**
 * @file augsys/mmap.h
 *
 * Memory mapping.
 */

#include "augsys/config.h"
#include "augsys/types.h"

#include "augext/mpool.h"

#include "augtypes.h"

#define AUG_MMAPRD 0x01
#define AUG_MMAPWR 0x02

struct aug_mmap {
    void* addr_;
    size_t len_;
};

AUGSYS_API void
aug_destroymmap(struct aug_mmap* mm);

AUGSYS_API struct aug_mmap*
aug_createmmap(aug_mpool* mpool, aug_fd fd, size_t offset, size_t len,
               int flags);

AUGSYS_API aug_result
aug_remmap(struct aug_mmap* mm, size_t offset, size_t len);

AUGSYS_API aug_result
aug_syncmmap(const struct aug_mmap* mm);

AUGSYS_API size_t
aug_mmapsize(const struct aug_mmap* mm);

AUGSYS_API unsigned
aug_granularity(void);

AUGSYS_API unsigned
aug_pagesize(void);

#endif /* AUGSYS_MMAP_H */
