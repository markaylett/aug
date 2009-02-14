/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
