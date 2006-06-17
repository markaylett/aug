/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_INET_H
#define AUGNET_INET_H

#include "augnet/config.h"

struct sockaddr_in;

AUGNET_API int
aug_tcplisten(const struct sockaddr_in* addr);

AUGNET_API struct sockaddr_in*
aug_parseinet(struct sockaddr_in* dst, const char* src);

AUGNET_API int
aug_setnodelay(int fd, int on);

#endif /* AUGNET_INET_H */
