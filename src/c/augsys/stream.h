/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_STREAM_H
#define AUGSYS_STREAM_H

#include "augsys/config.h"
#include "augsys/types.h"

#include "augext/stream.h"
#include "augext/mpool.h"

AUGSYS_API aug_stream*
aug_createfstream(aug_mpool* mpool, aug_fd fd);

AUGSYS_API aug_stream*
aug_createsstream(aug_mpool* mpool, aug_sd sd);

#endif /* AUGSYS_STREAM_H */
