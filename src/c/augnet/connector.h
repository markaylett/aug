/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_CONNECTOR_H
#define AUGNET_CONNECTOR_H

#include "augnet/config.h"

#include "augsys/socket.h"

typedef struct aug_connector_* aug_connector_t;

AUGNET_API aug_connector_t
aug_createconnector(const char* host, const char* serv);

AUGNET_API int
aug_destroyconnector(aug_connector_t ctor);

/**
   A blocking socket will be returned when the established flag is set.  Use
   aug_setnonblock() to change to non-blocking socket.
*/

AUGNET_API int
aug_tryconnect(aug_connector_t ctor, struct aug_endpoint* ep, int* est);

#endif /* AUGNET_CONNECTOR_H */
