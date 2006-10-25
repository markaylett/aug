/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#if defined(__CYGWIN__)
# undef _POSIX_SOURCE /* u_short */
#endif
#define AUGNET_BUILD
#include "augnet/inet.h"

static const char rcsid[] = "$Id$";

#include "augsys/errinfo.h"
#include "augsys/socket.h"
#include "augsys/string.h" /* aug_strlcpy() */
#include "augsys/unistd.h" /* aug_close() */

#if !defined(_WIN32)
# include <alloca.h>
# include <netinet/tcp.h>
#else /* _WIN32 */
# include <malloc.h>
#endif /* _WIN32 */

#include <string.h>        /* strchr() */

AUGNET_API int
aug_tcpconnect(const char* host, const char* serv, struct aug_endpoint* ep)
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

        ep->len_ = (socklen_t)res->ai_addrlen;
        memcpy(&ep->un_, res->ai_addr, res->ai_addrlen);

        if (0 == aug_connect(fd, ep))
            break; /* Success. */

        if (-1 == aug_close(fd)) /* Ignore this one. */
            goto fail;

    } while ((res = res->ai_next));

    if (!res) /* errno set from final aug_connect(). */
        goto fail;

    aug_freeaddrinfo(save);
    return fd;

 fail:
    aug_freeaddrinfo(save);
    return -1;
}

AUGNET_API int
aug_tcplisten(const char* host, const char* serv, struct aug_endpoint* ep)
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

        ep->len_ = (socklen_t)res->ai_addrlen;
        memcpy(&ep->un_, res->ai_addr, res->ai_addrlen);

        if (0 == aug_bind(fd, ep))
            break; /* Success. */

        if (-1 == aug_close(fd)) /* Bind error, close and try next one. */
            goto fail1;

    } while ((res = res->ai_next));

    if (!res) /* errno from final aug_socket() or aug_bind(). */
        goto fail1;

    if (-1 == aug_listen(fd, SOMAXCONN))
        goto fail2;

    aug_freeaddrinfo(save);
    return fd;

 fail2:
    aug_close(fd);

 fail1:
    aug_freeaddrinfo(save);
    return -1;
}

AUGNET_API int
aug_udpclient(const char* host, const char* serv, struct aug_endpoint* ep)
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

    ep->len_ = (socklen_t)res->ai_addrlen;
    memcpy(&ep->un_, res->ai_addr, res->ai_addrlen);

    aug_freeaddrinfo(save);
    return fd;

 fail:
    aug_freeaddrinfo(save);
    return -1;
}

AUGNET_API int
aug_udpconnect(const char* host, const char* serv, struct aug_endpoint* ep)
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

        ep->len_ = (socklen_t)res->ai_addrlen;
        memcpy(&ep->un_, res->ai_addr, res->ai_addrlen);

        if (0 == aug_connect(fd, ep))
            break; /* Success. */

        if (-1 == aug_close(fd)) /* Ignore this one. */
            goto fail;

    } while ((res = res->ai_next));

    if (!res) /* errno set from final aug_connect() */
        return -1;

    aug_freeaddrinfo(save);
    return fd;

 fail:
    aug_freeaddrinfo(save);
    return -1;
}

AUGNET_API int
aug_udpserver(const char* host, const char* serv, struct aug_endpoint* ep)
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

        ep->len_ = (socklen_t)res->ai_addrlen;
        memcpy(&ep->un_, res->ai_addr, res->ai_addrlen);

        if (0 == aug_bind(fd, ep))
            break; /* Success. */

        if (-1 == aug_close(fd)) /* bind error, close and try next one */
            goto fail;

    } while ((res = res->ai_next));

    if (!res) /* errno from final aug_socket() or aug_bind(). */
        return -1;

    aug_freeaddrinfo(save);
    return fd;

 fail:
    aug_freeaddrinfo(save);
    return -1;
}

AUGNET_API struct aug_hostserv*
aug_parsehostserv(const char* src, struct aug_hostserv* dst)
{
    size_t len;
    char* serv;

    aug_strlcpy(dst->data_, src, sizeof(dst->data_));

    /* Locate host and serv separator. */

    if (!(serv = strrchr(dst->data_, ':'))) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("missing separator '%s'"), src);
        return NULL;
    }

    /* Calculate length of host part. */

    len = serv - dst->data_;

    /* Ensure host and serv parts exists. */

    if (!len) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("missing host part '%s'"), src);
        return NULL;
    }

    if ('\0' == *++serv) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("missing service part '%s'"), src);
        return NULL;
    }

    /* The host part of an ipv6 address may be contained within square
       brackets. */

    if ('[' == dst->data_[0]) {

        if (']' != dst->data_[len - 1]) {
            aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                           AUG_MSG("unmatched brackets '%s'"), src);
            return NULL;
        }

        len -= 2;
        dst->host_ = dst->data_ + 1;
    } else
        dst->host_ = dst->data_;

    dst->host_[len] = '\0';
    dst->serv_ = serv;
    return dst;
}

AUGNET_API int
aug_setnodelay(int fd, int on)
{
    return aug_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
}
