/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_TCPCONNECT_H
#define AUGNET_TCPCONNECT_H

/**
 * @file augnet/tcpconnect.h
 *
 * Non-blocking TCP establishment.
 */

#include "augnet/config.h"

#include "augsys/socket.h"

typedef struct aug_tcpconnect_* aug_tcpconnect_t;

AUGNET_API aug_tcpconnect_t
aug_createtcpconnect(const char* host, const char* serv);

AUGNET_API int
aug_destroytcpconnect(aug_tcpconnect_t conn);

/**
 * Attempt to establish connection without blocking.
 *
 * If a connection was established, then @a est flag will be set and a
 * non-blocking socket returned.
 *
 * Otherwise, the returned descriptor should be polled for #AUG_FDEVENTCONN
 * events before retrying aug_tryconnect().
 *
 * The descriptor returned between calls to aug_tryconnect() may change; this
 * occurs as the implementation cycles through the addresses associated with
 * the host.  All but the established descriptor will be closed by the
 * aug_tcpconnect_t instance.
 */

/* FIXME: change to aug_bool. */

AUGNET_API aug_sd
aug_tryconnect(aug_tcpconnect_t conn, struct aug_endpoint* ep, int* est);

#endif /* AUGNET_TCPCONNECT_H */
