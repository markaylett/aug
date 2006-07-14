/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#if defined(__CYGWIN__)
# undef _POSIX_SOURCE /* u_short */
#endif
#define AUGNET_BUILD
#include "augnet/inet.h"

static const char rcsid[] = "$Id:$";

#include "augutil/conv.h"

#include "augsys/errinfo.h"
#include "augsys/inet.h"   /* aug_inetaton() */
#include "augsys/socket.h"
#include "augsys/unistd.h" /* aug_close() */

#if !defined(_WIN32)
# include <alloca.h>
# include <netdb.h>        /* gethostbyname() */
# include <netinet/in.h>
# include <netinet/tcp.h>
#else /* _WIN32 */
# include <malloc.h>
#endif /* _WIN32 */

#include <string.h>        /* strchr() */

AUGNET_API int
aug_tcplisten(const struct sockaddr* addr)
{
    int fd = aug_socket(addr->sa_family, SOCK_STREAM, 0);
    if (-1 == fd)
        return -1;

    if (-1 == aug_setreuseaddr(fd, 1))
        goto fail;

    if (-1 == aug_bind(fd, addr, AUG_INETLEN(addr->sa_family)))
        goto fail;

    if (-1 == aug_listen(fd, SOMAXCONN))
        goto fail;

    return fd;

 fail:
    aug_close(fd);
    return -1;
}

AUGNET_API struct sockaddr*
aug_parseinet(union aug_sockunion* dst, const char* src)
{
    size_t len;
    unsigned short nport;
    char* host;

    /* Locate host and port separator. */

    const char* port = strchr(src, ':');
    if (NULL == port)
        goto fail;

    /* Calculate length of host part. */

    len = port - src;

    /* Ensure host and port parts exists. */

    if (1 > len || '\0' == *++port)
        goto fail;

    /* Parse port value. */

    if (-1 == aug_strtous(&nport, port, 10))
        goto fail;

    /* Create null-terminated host string. */

    host = alloca(len + 1);
    memcpy(host, src, len);
    host[len] = '\0';

    /* Try to resolve dotted addresss notation first. */

    if (!aug_inetpton(AF_INET, host, &dst->sin_.sin_addr)) {

        /* Attempt to resolve host using DNS. */

        struct hostent* answ = gethostbyname(host);
        if (!answ || !answ->h_addr_list[0]) {

            aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXIST,
                           AUG_MSG("failed to resolve address '%s'"), src);
            return NULL;
        }

        memcpy(&dst->sin_.sin_addr, answ->h_addr_list[0],
               sizeof(dst->sin_.sin_addr));
    }

    dst->sin_.sin_family = AF_INET;
    dst->sin_.sin_port = htons(nport);
    return &dst->sa_;

 fail:
    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                   AUG_MSG("invalid address '%s'"), src);
    return NULL;
}

AUGNET_API int
aug_setnodelay(int fd, int on)
{
    return aug_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
}
