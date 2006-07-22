/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/base.h"
#include "augsys/errinfo.h"

#include <errno.h>
#include <unistd.h>

static int
tofd_(int fd)
{
    if (-1 == aug_openfd(fd, aug_posixdriver())) {
        close(fd);
        return -1;
    }

    return fd;
}

AUGSYS_API int
aug_socket(int domain, int type, int protocol)
{
    int fd = socket(domain, type, protocol);
    if (-1 == fd) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    return tofd_(fd);
}

AUGSYS_API int
aug_accept(int s, struct aug_endpoint* ep)
{
    int fd;
    ep->len_ = AUG_MAXADDRLEN;
    fd = accept(s,  &ep->un_.all_, &ep->len_);
    if (-1 == fd) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    return tofd_(fd);
}

AUGSYS_API int
aug_bind(int s, const struct aug_endpoint* ep)
{
    if (-1 == bind(s, &ep->un_.all_, ep->len_)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_connect(int s, const struct aug_endpoint* ep)
{
    if (-1 == connect(s, &ep->un_.all_, ep->len_)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API struct aug_endpoint*
aug_getpeername(int s, struct aug_endpoint* ep)
{
    if (-1 == getpeername(s, &ep->un_.all_, &ep->len_)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return NULL;
    }

    return ep;
}

AUGSYS_API struct aug_endpoint*
aug_getsockname(int s, struct aug_endpoint* ep)
{
    if (-1 == getsockname(s, &ep->un_.all_, &ep->len_)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return NULL;
    }

    return ep;
}

AUGSYS_API int
aug_listen(int s, int backlog)
{
    if (-1 == listen(s, backlog)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API ssize_t
aug_recv(int s, void* buf, size_t len, int flags)
{
    ssize_t ret;
    if (-1 == (ret = recv(s, buf, len, flags)))
        aug_setposixerrinfo(__FILE__, __LINE__, errno);

    return ret;
}

AUGSYS_API ssize_t
aug_recvfrom(int s, void* buf, size_t len, int flags, struct aug_endpoint* ep)
{
    ssize_t ret;
    ep->len_ = AUG_MAXADDRLEN;
    if (-1 == (ret = recvfrom(s, buf, len, flags, &ep->un_.all_, &ep->len_)))
        aug_setposixerrinfo(__FILE__, __LINE__, errno);

    return ret;
}

AUGSYS_API ssize_t
aug_send(int s, const void* buf, size_t len, int flags)
{
    ssize_t ret;
    if (-1 == (ret = send(s, buf, len, flags)))
        aug_setposixerrinfo(__FILE__, __LINE__, errno);

    return ret;
}

AUGSYS_API ssize_t
aug_sendto(int s, const void* buf, size_t len, int flags,
           const struct aug_endpoint* ep)
{
    ssize_t ret;
    if (-1 == (ret = sendto(s, buf, len, flags, &ep->un_.all_, ep->len_)))
        aug_setposixerrinfo(__FILE__, __LINE__, errno);

    return ret;
}

AUGSYS_API int
aug_getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen)
{
    if (-1 == getsockopt(s, level, optname, optval, optlen)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_setsockopt(int s, int level, int optname, const void* optval,
               socklen_t optlen)
{
    if (-1 == setsockopt(s, level, optname, optval, optlen)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_shutdown(int s, int how)
{
    if (-1 == shutdown(s, how)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_socketpair(int domain, int type, int protocol, int sv[2])
{
    if (-1 == socketpair(domain, type, protocol, sv)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    if (-1 == aug_openfds(sv, aug_posixdriver())) {
        close(sv[0]);
        close(sv[1]);
        return -1;
    }

    return 0;
}

AUGSYS_API char*
aug_inetntop(const struct aug_ipaddr* src, char* dst, socklen_t size)
{
    const char* ret = inet_ntop(src->family_, &src->un_, dst, size);
    if (!ret) {
      aug_setposixerrinfo(__FILE__, __LINE__, errno);
      return NULL;
    }
    return dst;
}

AUGSYS_API struct aug_ipaddr*
aug_inetpton(int af, const char* src, struct aug_ipaddr* dst)
{
    int ret = inet_pton(af, src, &dst->un_);
    if (ret < 0) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return NULL;
    } else if (0 == ret) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid address: %s"), src);
        return NULL;
    }

    dst->family_ = af;
    return dst;
}

AUGSYS_API void
aug_freeaddrinfo(struct addrinfo* res)
{
    freeaddrinfo(res);
}

AUGSYS_API int
aug_getaddrinfo(const char* host, const char* serv,
                const struct addrinfo* hints, struct addrinfo** res)
{
    int ret = getaddrinfo(host, serv, hints, res);
    if (0 != ret) {
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCPOSIX, ret,
                       gai_strerror(ret));
        return -1;
    }
    return 0;
}
