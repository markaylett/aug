/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_OBJECT_H
#define AUGNET_OBJECT_H

#include "augnet/config.h"

#include "augsys/muxer.h"

#include "augext/chan.h"
#include "augext/mpool.h"

struct ssl_ctx_st;

/**
 * Create a client socket channel.
 *
 * Will notify @ref aug_chandler of connection establishment when handshake is
 * complete.
 */

AUGNET_API aug_chan*
aug_createclient(aug_mpool* mpool, aug_muxer_t muxer, const char* host,
                 const char* serv, struct ssl_ctx_st* sslctx);

/**
 * Create a server socket channel.
 *
 * Must be passed a non-blocking socket.  If successful, will assume
 * responsibility for calling aug_sclose() on socket.
 *
 * Will notify @ref aug_chandler of connection establishment whenever a new
 * connection is accepted.
 */

AUGNET_API aug_chan*
aug_createserver(aug_mpool* mpool, aug_muxer_t muxer, aug_sd sd,
                 struct ssl_ctx_st* sslctx);

/**
 * Create a plain socket channel.
 *
 * Must be passed a non-blocking socket.  If successful, will assume
 * responsibility for calling aug_sclose() on socket.
 *
 * Assumes that @ref aug_chandler has already been notified of connection
 * establishment for @a id.
 */

AUGNET_API aug_chan*
aug_createplain(aug_mpool* mpool, aug_muxer_t muxer, unsigned id, aug_sd sd,
                unsigned short mask);

#endif /* AUGNET_OBJECT_H */
