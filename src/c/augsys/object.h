/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_OBJECT_H
#define AUGSYS_OBJECT_H

#include "augsys/muxer.h"

#include "augctx/mpool.h"

#include "augext/chan.h"

AUGSYS_API aug_chan*
aug_createfile(aug_mpool* mpool, aug_fd fd, const char* name,
               aug_muxer_t muxer);

#endif /* AUGSYS_OBJECT_H */
