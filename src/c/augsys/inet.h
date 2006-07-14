/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_INET_H
#define AUGSYS_INET_H

#include "augsys/config.h"

#if !defined(_WIN32)
# include <netinet/in.h>
# include <arpa/inet.h>
#else /* _WIN32 */
# include <winsock2.h>
# include <ws2tcpip.h>
#endif /* _WIN32 */

#define AUG_INETLEN(x) (AF_INET6 == x \
                        ? sizeof(struct sockaddr_in6) \
                        : sizeof(struct sockaddr_in))

AUGSYS_API int
aug_inetpton(int af, const char* src, void* dst);

#endif /* AUGSYS_INET_H */
