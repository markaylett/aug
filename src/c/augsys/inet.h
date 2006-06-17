/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_INET_H
#define AUGSYS_INET_H

#include "augsys/config.h"

#if !defined(_WIN32)
# include <netinet/in.h>
# include <arpa/inet.h>
#else // _WIN32
# include <winsock2.h>
#endif // _WIN32

AUGSYS_API int
aug_inetaton(const char *cp, struct in_addr *addr);

#endif /* AUGSYS_INET_H */
