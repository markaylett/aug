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

#include "augsys/file.h"

struct aug_errinfo;
struct ssl_st;

AUGNET_API void
aug_setsslerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                  unsigned long err);

AUGNET_API aug_file*
aug_attachsslclient(aug_mpool* mpool, aug_muxer_t muxer, aug_sd sd,
                    struct ssl_st* ssl);

AUGNET_API aug_file*
aug_attachsslserver(aug_mpool* mpool, aug_muxer_t muxer, aug_sd sd,
                    struct ssl_st* ssl);

#endif /* AUGNET_SSL_H */
