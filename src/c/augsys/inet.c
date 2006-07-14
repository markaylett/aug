/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/inet.h"

static const char rcsid[] = "$Id:$";

#include "augsys/errinfo.h"

#include <string.h> /* memcpy() */

AUGSYS_API int
aug_inetpton(int af, const char* src, void* dst)
{
#if !defined(_WIN32)
    return inet_pton(af, src, dst);
#else /* _WIN32 */

    if (AF_INET == af) {

        struct sockaddr_in sa = { 0 };
		INT len = sizeof(sa);
        sa.sin_family = AF_INET6;

		if (SOCKET_ERROR
            == WSAStringToAddress((LPTSTR)src, af, NULL,
                                  (struct sockaddr*)&sa, &len)) {
            aug_setwin32errinfo(__FILE__, __LINE__, WSAGetLastError());
            return -1;
        }

        memcpy(dst, &sa.sin_addr, sizeof(sa));

    } else if (AF_INET6 == af) {

        struct sockaddr_in6 sa = { 0 };
		INT len = sizeof(sa);
        sa.sin6_family = AF_INET6;

		if (SOCKET_ERROR
            == WSAStringToAddress((LPTSTR)src, af, NULL,
                                  (struct sockaddr*)&sa, &len)) {
            aug_setwin32errinfo(__FILE__, __LINE__, WSAGetLastError());
            return -1;
        }

        memcpy(dst, &sa.sin6_addr, sizeof(sa));

    } else {
        aug_setwin32errinfo(__FILE__, __LINE__, WSAEAFNOSUPPORT);
        return -1;
    }

    return 0;
#endif /* _WIN32 */
}
