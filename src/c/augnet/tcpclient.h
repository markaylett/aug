/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_TCPCLIENT_H
#define AUGNET_TCPCLIENT_H

/**
 * @file augnet/tcpclient.h
 *
 * Non-blocking TCP establishment.
 */

#include "augnet/config.h"

#include "augsys/socket.h"

typedef struct aug_tcpclient_* aug_tcpclient_t;

AUGNET_API aug_tcpclient_t
aug_createtcpclient(const char* host, const char* serv);

AUGNET_API int
aug_destroytcpclient(aug_tcpclient_t client);

/**
 * Attempt to establish connection without blocking.
 *
 * If a connection was established, then @a est flag will be set and a
 * blocking socket returned.  Use aug_ssetnonblock() to change to non-blocking
 * socket.
 *
 * Otherwise, the returned descriptor should be polled before retrying
 * aug_tryconnect().
 *
 * The descriptor returned between calls to aug_tryconnect() may change; this
 * occurs as the implementation cycles through the addresses associated with
 * the host.
 */

AUGNET_API aug_sd
aug_tryconnect(aug_tcpclient_t client, struct aug_endpoint* ep, int* est);

#endif /* AUGNET_TCPCLIENT_H */
