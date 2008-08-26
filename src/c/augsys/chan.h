/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_CHAN_H
#define AUGSYS_CHAN_H

#include "augsys/muxer.h"

#include "augctx/mpool.h"

#include "augext/chan.h"

AUGSYS_API aug_bool
aug_safeestab(aug_chan* chan, aug_chandler* handler, unsigned id,
              aug_stream* ob, unsigned parent);

AUGSYS_API aug_bool
aug_safeready(aug_chan* chan, aug_chandler* handler, unsigned id,
              aug_stream* ob, unsigned short events);

AUGSYS_API aug_chan*
aug_createfile(aug_mpool* mpool, aug_fd fd, const char* name,
               aug_muxer_t muxer);

#endif /* AUGSYS_CHAN_H */
