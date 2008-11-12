/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_CHAN_H
#define AUGSYS_CHAN_H

#include "augsys/muxer.h"

#include "augctx/mpool.h"

#include "augext/chan.h"

AUGSYS_API aug_chan*
aug_createfile(aug_mpool* mpool, const char* name, aug_muxer_t muxer,
               aug_fd fd);

#endif /* AUGSYS_CHAN_H */
