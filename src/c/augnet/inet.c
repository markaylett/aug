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
aug_tcpconnect(const char* host, const char* serv, struct aug_sockaddr* addr)
{
    int fd;
    struct addrinfo hints, * res, * save;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (-1 == aug_getaddrinfo(host, serv, &hints, &res))
        return -1;

    save = res;

    do {
        fd = aug_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (-1 == fd)
            continue; /* Ignore this one. */

        if (0 == aug_connect(fd, res->ai_addr, res->ai_addrlen))
            break; /* Success. */

        if (-1 == aug_close(fd)) /* Ignore this one. */
            goto fail;

    } while ((res = res->ai_next));

    if (!res) /* errno set from final aug_connect(). */
        goto fail;

    addr->addrlen_ = res->ai_addrlen;
    memcpy(addr->un_.data_, res->ai_addr, res->ai_addrlen);

    aug_freeaddrinfo(save);
    return fd;

 fail:
    aug_freeaddrinfo(save);
    return -1;
}

AUGNET_API int
aug_tcplisten(const char* host, const char* serv, struct aug_sockaddr* addr)
{
    int fd;
    struct addrinfo hints, * res, * save;

    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (-1 == aug_getaddrinfo(host, serv, &hints, &res))
        return -1;

    save = res;

    do {
        fd = aug_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (-1 == fd)
            continue; /* Error, try next one. */

        if (-1 == aug_setreuseaddr(fd, 1))
            goto fail2;

        if (0 == aug_bind(fd, res->ai_addr, res->ai_addrlen))
            break; /* Success. */

        if (-1 == aug_close(fd)) /* Bind error, close and try next one. */
            goto fail1;

    } while ((res = res->ai_next));

    if (!res) /* errno from final aug_socket() or aug_bind(). */
        goto fail1;

    if (-1 == aug_listen(fd, SOMAXCONN))
        goto fail2;

    addr->addrlen_ = res->ai_addrlen;
    memcpy(addr->un_.data_, res->ai_addr, res->ai_addrlen);

    aug_freeaddrinfo(save);
    return fd;

 fail2:
    aug_close(fd);

 fail1:
    aug_freeaddrinfo(save);
    return -1;
}

AUGNET_API int
aug_udpclient(const char* host, const char* serv, struct aug_sockaddr* addr)
{
    int fd;
    struct addrinfo hints, * res, * save;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (-1 == aug_getaddrinfo(host, serv, &hints, &res))
        return -1;

    save = res;

    do {
        fd = aug_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (-1 != fd)
            break; /* Success. */

    } while ((res = res->ai_next));

    if (!res) /* errno set from final aug_socket(). */
        goto fail;

    addr->addrlen_ = res->ai_addrlen;
    memcpy(addr->un_.data_, res->ai_addr, res->ai_addrlen);

    aug_freeaddrinfo(save);
    return fd;

 fail:
    aug_freeaddrinfo(save);
    return -1;
}

AUGNET_API int
aug_udpconnect(const char* host, const char* serv, struct aug_sockaddr* addr)
{
    int fd;
    struct addrinfo hints, * res, * save;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (-1 == aug_getaddrinfo(host, serv, &hints, &res))
        return -1;

    save = res;

    do {
        fd = aug_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (-1 == fd)
            continue; /* Ignore this one. */

        if (0 == aug_connect(fd, res->ai_addr, res->ai_addrlen))
            break; /* Success. */

        if (-1 == aug_close(fd)) /* Ignore this one. */
            goto fail;

    } while ((res = res->ai_next));

    if (!res) /* errno set from final aug_connect() */
        return -1;

    addr->addrlen_ = res->ai_addrlen;
    memcpy(addr->un_.data_, res->ai_addr, res->ai_addrlen);

    aug_freeaddrinfo(save);
    return fd;

 fail:
    aug_freeaddrinfo(save);
    return -1;
}

AUGNET_API int
aug_udpserver(const char* host, const char* serv, struct aug_sockaddr* addr)
{
    int fd;
    struct addrinfo hints, * res, * save;

    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (-1 == aug_getaddrinfo(host, serv, &hints, &res))
        return -1;

    save = res;

    do {
        fd = aug_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (-1 == fd)
            continue; /* Error, try next one. */

        if (0 == aug_bind(fd, res->ai_addr, res->ai_addrlen))
            break; /* Success. */

        if (-1 == aug_close(fd)) /* bind error, close and try next one */
            goto fail;

    } while ((res = res->ai_next));

    if (!res) /* errno from final aug_socket() or aug_bind(). */
        return -1;

    addr->addrlen_ = res->ai_addrlen;
    memcpy(addr->un_.data_, res->ai_addr, res->ai_addrlen);

    aug_freeaddrinfo(save);
    return fd;

 fail:
    aug_freeaddrinfo(save);
    return -1;
}

AUGNET_API struct sockaddr*
aug_parseinet(struct aug_sockaddr* dst, const char* src)
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

    if (!aug_inetpton(AF_INET, host, &dst->un_.sin_.sin_addr)) {

        /* Attempt to resolve host using DNS. */

        struct hostent* answ = gethostbyname(host);
        if (!answ || !answ->h_addr_list[0]) {

            aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXIST,
                           AUG_MSG("failed to resolve address '%s'"), src);
            return NULL;
        }

        dst->addrlen_ = sizeof(dst->un_.sin_.sin_addr);
        memcpy(&dst->un_.sin_.sin_addr, answ->h_addr_list[0],
               sizeof(dst->un_.sin_.sin_addr));
    }

    dst->un_.sin_.sin_family = AF_INET;
    dst->un_.sin_.sin_port = htons(nport);
    return &dst->un_.sa_;

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
