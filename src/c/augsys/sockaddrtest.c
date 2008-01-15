/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"

#include <stdio.h>

#define HOST_ "localhost"
#define SERV_ "5000"

static char*
tostr_(char* dst, const struct aug_endpoint* ep)
{
    struct aug_inetaddr addr;
    char buf[AUG_MAXHOSTNAMELEN + 1];
    size_t size;

    aug_getinetaddr(ep, &addr);
    aug_inetntop(&addr, buf, sizeof(buf));
    size = strlen(buf);

    // []:65536\0

    switch (ep->un_.family_) {
    case AF_INET:
        sprintf(dst, "%s:%d", buf, (int)aug_ntoh16(ep->un_.all_.port_));
        break;
#if HAVE_IPV6
    case AF_INET6:
        sprintf(dst, "[%s]:%d", buf, (int)aug_ntoh16(ep->un_.all_.port_));
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
    hints.ai_family = AF_UNSPEC;
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
        tostr_(buf, &ep);
        printf("%s\n", buf);

    } while ((res = res->ai_next));
    aug_destroyaddrinfo(save);
    return 0;
}
