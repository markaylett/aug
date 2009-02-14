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
#include "augsys/time.h"

#include "augctx/base.h"
#include "augctx/clock.h" /* aug_createclock() */
#include "augctx/errinfo.h"

#include <errno.h>        /* ENOMEM */

struct aug_hires_ {
    aug_mpool* mpool_;
    aug_clock* clock_;
    struct timeval start_;
};

AUGUTIL_API aug_hires_t
aug_createhires(aug_mpool* mpool)
{
    aug_clock* clock;
    aug_hires_t hires;

    /* Create clock rather than reuse the tlx version to ensure high
       precision. */

    if (!(clock = aug_createclock(mpool, 0)))
        return NULL;

    if (!(hires = aug_allocmem(mpool, sizeof(struct aug_hires_))))
        goto fail1;

    hires->mpool_ = mpool;
    hires->clock_ = clock;

    if (AUG_ISFAIL(aug_gettimeofday(clock, &hires->start_)))
        goto fail2;

    /* Success. */

    aug_retain(mpool);
    return hires;

 fail2:
    aug_freemem(mpool, hires);
 fail1:
    aug_release(clock);
    return NULL;
}

AUGUTIL_API void
aug_destroyhires(aug_hires_t hires)
{
    aug_mpool* mpool = hires->mpool_;
    aug_clock* clock = hires->clock_;
    aug_freemem(mpool, hires);
    aug_release(clock);
    aug_release(mpool);
}

AUGUTIL_API aug_result
aug_resethires(aug_hires_t hires)
{
    return aug_gettimeofday(hires->clock_, &hires->start_);
}

AUGUTIL_API double*
aug_elapsed(aug_hires_t hires, double* sec)
{
    struct timeval now;
    if (AUG_ISFAIL(aug_gettimeofday(hires->clock_, &now)))
        return NULL;
    aug_tvsub(&now, &hires->start_);
    *sec = (double)now.tv_sec + ((double)now.tv_usec / 1000000.0);
    return sec;
}
