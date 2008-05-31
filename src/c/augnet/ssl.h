/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_SSL_H
#define AUGNET_SSL_H

/**
 * @file augnet/ssl.h
 *
 * SSL support.
 */

#include "augnet/config.h"

#include "augsys/muxer.h"

#include "augext/chan.h"
#include "augext/mpool.h"

struct aug_errinfo;
struct ssl_st;

AUGNET_API void
aug_setsslerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                  unsigned long err);

/**
 * Create an ssl client channel.
 *
 * Must be passed a non-blocking socket.  If successful, will assume
 * responsibility for calling aug_sclose() on socket.
 */

AUGNET_API aug_chan*
aug_createsslclient(aug_mpool* mpool, unsigned id, aug_muxer_t muxer,
                    aug_sd sd, unsigned short mask, struct ssl_st* ssl);

/**
 * Create an ssl server channel.
 *
 * Must be passed a non-blocking socket.  If successful, will assume
 * responsibility for calling aug_sclose() on socket.
 */

AUGNET_API aug_chan*
aug_createsslserver(aug_mpool* mpool, unsigned id, aug_muxer_t muxer,
                    aug_sd sd, unsigned short mask, struct ssl_st* ssl);

#endif /* AUGNET_SSL_H */
