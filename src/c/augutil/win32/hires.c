/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
#include "augsys/windows.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <errno.h>  /* ENOMEM */

struct aug_hires_ {
    aug_mpool* mpool_;
    LARGE_INTEGER freq_, start_;
};

AUGUTIL_API aug_hires_t
aug_createhires(aug_mpool* mpool)
{
    aug_hires_t hires = aug_allocmem(mpool, sizeof(struct aug_hires_));
    if (!hires)
        return NULL;

    hires->mpool_ = mpool;

    if (!QueryPerformanceFrequency(&hires->freq_)
        || !QueryPerformanceCounter(&hires->start_)) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        aug_freemem(mpool, hires);
        return NULL;
    }

    aug_retain(mpool);
    return hires;
}

AUGUTIL_API void
aug_destroyhires(aug_hires_t hires)
{
    aug_mpool* mpool = hires->mpool_;
    aug_freemem(mpool, hires);
    aug_release(mpool);
}

AUGUTIL_API aug_result
aug_resethires(aug_hires_t hires)
{
    if (!QueryPerformanceCounter(&hires->start_)) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS;
}

AUGUTIL_API double*
aug_elapsed(aug_hires_t hires, double* sec)
{
    LARGE_INTEGER now;
    if (!QueryPerformanceCounter(&now)) {
        aug_setwin32errinfo(aug_tlerr, __FILE__, __LINE__, GetLastError());
        return NULL;
    }

    /* Ticks relative to start. */

    now.QuadPart -= hires->start_.QuadPart;
    *sec = (double)now.QuadPart / (double)hires->freq_.QuadPart;
    return sec;
}
