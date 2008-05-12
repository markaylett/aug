/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_OBJECT_H
#define AUGNET_OBJECT_H

#include "augnet/config.h"

#include "augsys/muxer.h"

#include "augctx/mpool.h"

#include "augob/channelob.h"

AUGNET_API aug_channelob*
aug_createclient(aug_mpool* mpool, const char* host, const char* serv,
                 aug_muxer_t muxer);

AUGNET_API aug_channelob*
aug_createserver(aug_mpool* mpool, aug_sd sd, aug_muxer_t muxer);

AUGNET_API aug_channelob*
aug_createplain(aug_mpool* mpool, aug_sd sd, aug_muxer_t muxer);

#endif /* AUGNET_OBJECT_H */
