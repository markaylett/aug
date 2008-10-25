/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_CHAN_H
#define AUGSYS_CHAN_H

#include "augsys/muxer.h"

#include "augctx/mpool.h"

#include "augext/chan.h"

AUGSYS_API void
aug_clearerror(aug_chandler* handler, aug_chan* chan,
               struct aug_errinfo* errinfo);

AUGSYS_API aug_bool
aug_clearestab(aug_chandler* handler, aug_chan* chan, unsigned parent);

/**
 * Error-info is assumed to be set accordingly, prior to calling
 * aug_clearready().
 */

AUGSYS_API aug_bool
aug_clearready(aug_chandler* handler, aug_chan* chan, unsigned short events);

AUGSYS_API aug_chan*
aug_createfile(aug_mpool* mpool, const char* name, aug_muxer_t muxer,
               aug_fd fd);

#endif /* AUGSYS_CHAN_H */
