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
#define AUGCTX_BUILD
#include "augctx/clock.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
# include <sys/time.h>
#else /* _WIN32 */
# include <sys/timeb.h> /* _ftime() */
# include <winsock2.h>  /* struct timeval */
# define ftime _ftime
# define timeb _timeb
#endif /* _WIN32 */

struct impl_ {
    aug_clock clock_;
    int refs_;
    aug_mpool* mpool_;
    long timezone_;
};

static void*
cast_(aug_clock* obj, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_clockid)) {
        aug_retain(obj);
        return obj;
    }
    return NULL;
}

static void
retain_(aug_clock* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, clock_, obj);
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
release_(aug_clock* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, clock_, obj);
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
}

static aug_result
gettimeofday_(aug_clock* obj, struct aug_timeval* tv)
{
#if !defined(_WIN32)
    /* FIXME: policy for setting errinfo? */
    struct timeval local;
    if (gettimeofday(&local, NULL) < 0)
        return -1;
    tv->tv_sec = (aug_time)local.tv_sec;
    tv->tv_usec = (aug_suseconds)local.tv_usec;
#else /* _WIN32 */
    struct timeb tb;
    ftime(&tb);
    tv->tv_sec = (aug_time)tb.time;
    tv->tv_usec = (aug_suseconds)(tb.millitm * 1000);
#endif /* _WIN32 */
    return 0;
}

static long
gettimezone_(aug_clock* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, clock_, obj);
    return impl->timezone_;
}

static const struct aug_clockvtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    gettimeofday_,
    gettimezone_
};

AUGCTX_API aug_clock*
aug_createclock(aug_mpool* mpool, long tz)
{
    struct impl_* impl = aug_allocmem(mpool, sizeof(struct impl_));
    if (!impl)
        return NULL;

    impl->clock_.vtbl_ = &vtbl_;
    impl->clock_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->timezone_ = tz;

    aug_retain(mpool);
    return &impl->clock_;
}
