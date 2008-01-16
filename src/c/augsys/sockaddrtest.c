/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"

#include <assert.h>
#include <stdio.h>

#define HOST_ "localhost"
#define SERV_ "5000"

#if defined(_WIN32)
# define snprintf _snprintf
#endif /* _WIN32 */

static char*
aug_endpointntop(const struct aug_endpoint* src, char* dst, socklen_t len)
{
    struct aug_inetaddr addr;
    char host[AUG_MAXHOSTNAMELEN + 1];
    const char* fmt;
    int ret;

    assert(src && dst && len);

    if (!aug_getinetaddr(src, &addr)
        || !aug_inetntop(&addr, host, sizeof(host)))
        return NULL;

    switch (src->un_.family_) {
    case AF_INET:
        fmt = "%s:%d";
        break;
#if HAVE_IPV6
    case AF_INET6:
        fmt = "[%s]:%d";
        break;
#endif /* HAVE_IPV6 */
    default:
#if !defined(_WIN32)
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, EAFNOSUPPORT);
#else /* _WIN32 */
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAEAFNOSUPPORT);
#endif /* _WIN32 */
        return NULL;
    }

    /* Null termination is _not_ guaranteed by snprintf(). */

    ret = snprintf(dst, len, fmt, host, (int)aug_ntoh16(src->un_.all_.port_));
    AUG_SNTRUNCF(dst, len, ret);

    if (ret < 0) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return NULL;
    }

    return dst;
}

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    struct addrinfo hints, * res, * save;

    aug_atexitinit(&errinfo);

    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if (-1 == aug_getaddrinfo(HOST_, SERV_, &hints, &res)) {
        aug_perrinfo(NULL, "aug_getaddrinfo() failed");
        return 1;
    }

    save = res;
    do {
        struct aug_endpoint ep;
        char buf[AUG_MAXHOSTSERVLEN + 1];

        aug_getendpoint(res, &ep);
        if (!aug_endpointntop(&ep, buf, sizeof(buf))) {
            aug_perrinfo(NULL, "aug_endpointntop() failed");
            aug_destroyaddrinfo(save);
            return 1;
        }

        printf("%s\n", buf);

    } while ((res = res->ai_next));
    return 0;
}
