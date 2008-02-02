/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/clock.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> /* tzset() */

#if defined(_WIN32)
# include <sys/timeb.h>
# include <windows.h>
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
        aug_free(mpool, impl);
        aug_release(mpool);
    }
}

static struct timeval*
gettimeofday_(aug_clock* obj, struct timeval* tv)
{
#if !defined(_WIN32)
    if (-1 == gettimeofday(tv, NULL))
        return NULL;
#else /* _WIN32 */
    struct _timeb tb;
    _ftime(&tb);
    tv->tv_sec = (long)tb.time;
    tv->tv_usec = tb.millitm * 1000;
#endif /* _WIN32 */
    return tv;
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
    struct impl_* impl;
    assert(mpool);

    if (!(impl = aug_malloc(mpool, sizeof(struct impl_))))
        return NULL;

    impl->clock_.vtbl_ = &vtbl_;
    impl->clock_.impl_ = NULL;
    impl->refs_ = 1;

    aug_retain(mpool);

    impl->mpool_ = mpool;
    impl->timezone_ = tz;

    return &impl->clock_;
}

#if !defined(_WIN32)
AUGCTX_API long*
aug_timezone(long* tz)
{
    tzset();
    *tz = timezone;
    return tz;
}
#else /* _WIN32 */
AUGCTX_API long*
aug_timezone(long* tz)
{
	TIME_ZONE_INFORMATION tzi;
    switch (GetTimeZoneInformation(&tzi)) {
    case TIME_ZONE_ID_INVALID:
    case TIME_ZONE_ID_UNKNOWN:
        return NULL;
    case TIME_ZONE_ID_STANDARD:
    case TIME_ZONE_ID_DAYLIGHT:
        break;
    }
    *tz = (tzi.Bias + tzi.StandardBias) * 60;
    return tz;
}
#endif /* _WIN32 */
