/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/muxer.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

static const char LABELS_[][4] = {
    "___",
    "R__",
    "_W_",
    "RW_",
    "__X",
    "R_X",
    "_WX",
    "RWX"
};

static const struct timeval NOWAIT_ = { 0, 0 };

#if !defined(_WIN32)
# include "augsys/posix/muxer.c"
#else /* _WIN32 */
# include "augsys/win32/muxer.c"
#endif /* _WIN32 */

AUGSYS_API aug_rint
aug_pollmdevents(aug_muxer_t muxer)
{
    return aug_waitmdevents(muxer, &NOWAIT_);
}

AUGSYS_API const char*
aug_eventlabel(unsigned short events)
{
    if (sizeof(LABELS_) / sizeof(LABELS_[0]) <= (size_t)events)
        events = 0;

    return LABELS_[events];
}
