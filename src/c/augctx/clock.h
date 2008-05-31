/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_CLOCK_H
#define AUGCTX_CLOCK_H

#include "augctx/config.h"

#include "augext/clock.h"
#include "augext/mpool.h"

AUGCTX_API aug_clock*
aug_createclock(aug_mpool* mpool, long tz);

#endif /* AUGCTX_CLOCK_H */
