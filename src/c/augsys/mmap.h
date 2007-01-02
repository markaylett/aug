/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_MMAP_H
#define AUGSYS_MMAP_H

#include "augsys/config.h"
#include "augsys/types.h"

#define AUG_MMAPRD 0x01
#define AUG_MMAPWR 0x02

struct aug_mmap_ {
    void* addr_;
    size_t len_;
};

AUGSYS_API int
aug_freemmap(struct aug_mmap_* mmap_);

AUGSYS_API struct aug_mmap_*
aug_createmmap(int fd, size_t offset, size_t len, int flags);

AUGSYS_API int
aug_remmap(struct aug_mmap_* mmap_, size_t offset, size_t len);

AUGSYS_API int
aug_syncmmap(struct aug_mmap_* mmap_);

AUGSYS_API size_t
aug_mmapsize(struct aug_mmap_* mmap_);

AUGSYS_API unsigned
aug_granularity(void);

AUGSYS_API unsigned
aug_pagesize(void);

#endif /* AUGSYS_MMAP_H */
