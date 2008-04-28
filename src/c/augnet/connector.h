/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_CONNECTOR_H
#define AUGNET_CONNECTOR_H

/**
 * @file augnet/connector.h
 *
 * Non-blocking TCP establishment.
 */

#include "augnet/config.h"

#include "augsys/socket.h"

typedef struct aug_connector_* aug_connector_t;

AUGNET_API aug_connector_t
aug_createconnector(const char* host, const char* serv);

AUGNET_API int
aug_destroyconnector(aug_connector_t ctor);

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
aug_tryconnect(aug_connector_t ctor, struct aug_endpoint* ep, int* est);

#endif /* AUGNET_CONNECTOR_H */
