/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_OBJECT_H
#define AUGSYS_OBJECT_H

#include "augsys/muxer.h"

#include "augctx/mpool.h"

#include "augob/channelob.h"

AUGSYS_API aug_channelob*
aug_createfile(aug_mpool* mpool, aug_fd fd, aug_muxer_t muxer);

AUGSYS_API aug_channelob*
aug_createsocket(aug_mpool* mpool, aug_sd sd, aug_muxer_t muxer);

#endif /* AUGSYS_OBJECT_H */
