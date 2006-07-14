/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_INET_H
#define AUGNET_INET_H

#include "augnet/config.h"

#include "augsys/inet.h"

union aug_sockunion {
	struct sockaddr sa_;
	struct sockaddr_in sin_;
	struct sockaddr_in6 sin6_;
};

AUGNET_API int
aug_tcplisten(const struct sockaddr* addr);

AUGNET_API struct sockaddr*
aug_parseinet(union aug_sockunion* dst, const char* src);

AUGNET_API int
aug_setnodelay(int fd, int on);

#endif /* AUGNET_INET_H */
