/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_SSL_H
#define AUGNET_SSL_H

#include "augnet/config.h"

AUGNET_API int
aug_setsslclient(int fd, void* ctx);

AUGNET_API int
aug_setsslserver(int fd, void* ctx);

#endif /* AUGNET_SSL_H */
