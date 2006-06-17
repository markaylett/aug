/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/base.h"

#include <unistd.h>

static int
tofd_(int fd)
{
    if (-1 == aug_openfd(fd, AUG_FDSOCK)) {
        close(fd);
        return -1;
    }

    return fd;
}

static int
tofds_(int sv[2])
{
    if (-1 == tofd_(sv[0])) {
        close(sv[1]);
        return -1;
    }

    if (-1 == tofd_(sv[1])) {
        aug_releasefd(sv[0]);
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_socket(int domain, int type, int protocol)
{
    int fd = socket(domain, type, protocol);
    if (-1 == fd)
        return -1;

    return tofd_(fd);
}

AUGSYS_API int
aug_accept(int s, struct sockaddr* addr, socklen_t* addrlen)
{
    int fd = accept(s, addr, addrlen);
    if (-1 == fd)
        return -1;

    return tofd_(fd);
}

AUGSYS_API int
aug_bind(int s, const struct sockaddr* addr, socklen_t addrlen)
{
    return bind(s, addr, addrlen);
}

AUGSYS_API int
aug_connect(int s, const struct sockaddr* addr, socklen_t addrlen)
{
    return connect(s, addr, addrlen);
}

AUGSYS_API int
aug_getpeername(int s, struct sockaddr* addr, socklen_t* addrlen)
{
    return getpeername(s, addr, addrlen);
}

AUGSYS_API int
aug_getsockname(int s, struct sockaddr* addr, socklen_t* addrlen)
{
    return getsockname(s, addr, addrlen);
}

AUGSYS_API int
aug_listen(int s, int backlog)
{
    return listen(s, backlog);
}

AUGSYS_API ssize_t
aug_recv(int s, void* buf, size_t len, int flags)
{
    return recv(s, buf, len, flags);
}

AUGSYS_API ssize_t
aug_recvfrom(int s, void* buf, size_t len, int flags, struct sockaddr* from,
             socklen_t* fromlen)
{
    return recvfrom(s, buf, len, flags, from, fromlen);
}

AUGSYS_API ssize_t
aug_send(int s, const void* buf, size_t len, int flags)
{
    return send(s, buf, len, flags);
}

AUGSYS_API ssize_t
aug_sendto(int s, const void* buf, size_t len, int flags,
           const struct sockaddr* to, socklen_t tolen)
{
    return sendto(s, buf, len, flags, to, tolen);
}

AUGSYS_API int
aug_getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen)
{
    return getsockopt(s, level, optname, optval, optlen);
}

AUGSYS_API int
aug_setsockopt(int s, int level, int optname, const void* optval,
               socklen_t optlen)
{
    return setsockopt(s, level, optname, optval, optlen);
}

AUGSYS_API int
aug_shutdown(int s, int how)
{
    return shutdown(s, how);
}

AUGSYS_API int
aug_socketpair(int domain, int type, int protocol, int sv[2])
{
    if (-1 == socketpair(domain, type, protocol, sv))
        return -1;

    return tofds_(sv);
}
