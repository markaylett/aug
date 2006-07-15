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
aug_accept(int s, struct sockaddr* addr, socklen_t* addrlen)
{
    int fd = accept(s, addr, addrlen);
    if (-1 == fd) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    return tofd_(fd);
}

AUGSYS_API int
aug_bind(int s, const struct sockaddr* addr, socklen_t addrlen)
{
    if (-1 == bind(s, addr, addrlen)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_connect(int s, const struct sockaddr* addr, socklen_t addrlen)
{
    if (-1 == connect(s, addr, addrlen)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_getpeername(int s, struct sockaddr* addr, socklen_t* addrlen)
{
    if (-1 == getpeername(s, addr, addrlen)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_getsockname(int s, struct sockaddr* addr, socklen_t* addrlen)
{
    if (-1 == getsockname(s, addr, addrlen)) {
        aug_setposixerrinfo(__FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
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
aug_recvfrom(int s, void* buf, size_t len, int flags, struct sockaddr* from,
             socklen_t* fromlen)
{
    ssize_t ret;
    if (-1 == (ret = recvfrom(s, buf, len, flags, from, fromlen)))
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
           const struct sockaddr* to, socklen_t tolen)
{
    ssize_t ret;
    if (-1 == (ret = sendto(s, buf, len, flags, to, tolen)))
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

AUGSYS_API int
aug_setreuseaddr(int s, int on)
{
    return aug_setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
}

AUGSYS_API int
aug_getsockaf(int s)
{
    union {
        struct sockaddr sa;
        char data[AUG_MAXSOCKADDR];
    } u;

    socklen_t len = AUG_MAXSOCKADDR;
    if (-1 == getsockname(s, &u.sa, &len))
        return -1;

    return u.sa.sa_family;
}
