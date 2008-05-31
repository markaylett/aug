/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_OBJECT_H
#define AUGNET_OBJECT_H

#include "augnet/config.h"

#include "augsys/muxer.h"

#include "augext/chan.h"
#include "augext/mpool.h"

struct ssl_st* ssl;

/**
 * Create a client socket channel.
 */

AUGNET_API aug_chan*
aug_createclient(aug_mpool* mpool, const char* host, const char* serv,
                 aug_muxer_t muxer, struct ssl_st* ssl);

/**
 * Create a server socket channel.
 *
 * Must be passed a non-blocking socket.  If successful, will assume
 * responsibility for calling aug_sclose() on socket.
 */

AUGNET_API aug_chan*
aug_createserver(aug_mpool* mpool, aug_muxer_t muxer, aug_sd sd,
                 struct ssl_st* ssl);

/**
 * Create a plain socket channel.
 *
 * Must be passed a non-blocking socket.  If successful, will assume
 * responsibility for calling aug_sclose() on socket.
 */

AUGNET_API aug_chan*
aug_createplain(aug_mpool* mpool, unsigned id, aug_muxer_t muxer, aug_sd sd,
                unsigned short mask);

#endif /* AUGNET_OBJECT_H */
