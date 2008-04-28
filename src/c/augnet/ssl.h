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

#include "augsys/types.h"

struct aug_errinfo;
struct ssl_st;

AUGNET_API void
aug_setsslerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                  unsigned long err);

AUGNET_API int
aug_setsslclient(aug_sd sd, struct ssl_st* ssl);

AUGNET_API int
aug_setsslserver(aug_sd sd, struct ssl_st* ssl);

#endif /* AUGNET_SSL_H */
