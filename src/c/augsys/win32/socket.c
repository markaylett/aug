/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/base.h"
#include "augsys/errno.h"

#include <io.h>

static int
tofd_(SOCKET h)
{
    int fd = _open_osfhandle(h, 0);
    if (-1 == fd) {
        errno = EINVAL;
        closesocket(h);
        return -1;
    }

    if (-1 == aug_openfd(fd, AUG_FDSOCK)) {
        close(fd);
        return -1;
    }

    return fd;
}

static int
tofds_(SOCKET hs, SOCKET hc, int sv[2])
{
    int s, c;
    if (-1 == (s = tofd_(hs))) {
        closesocket(hc);
        return -1;
    }

    if (-1 == (c = tofd_(hc))) {
        aug_releasefd(s);
        return -1;
    }

	sv[0] = s;
	sv[1] = c;
    return 0;
}

static int
toerror_(void)
{
    aug_maperror(WSAGetLastError());
    return -1;
}

static int
streampair_(int protocol, int sv[2])
{
	socklen_t len;
	SOCKADDR_IN addr = { 0 };
	SOCKET l, c, s;

	if (INVALID_SOCKET == (l = socket(AF_INET, SOCK_STREAM, protocol)))
        goto fail1;

	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (SOCKET_ERROR == bind(l, (struct sockaddr*)&addr, sizeof(addr)))
		goto fail2;

	if (SOCKET_ERROR == listen(l, 1))
		goto fail2;

	len = sizeof(addr);
	if (SOCKET_ERROR == getsockname(l, (struct sockaddr*)&addr, &len))
		goto fail2;

	if (INVALID_SOCKET == (c = socket(AF_INET, SOCK_STREAM, protocol)))
		goto fail2;

	if (SOCKET_ERROR == connect(c, (struct sockaddr*)&addr, sizeof(addr)))
		goto fail3;

	len = sizeof(addr);
	if (INVALID_SOCKET == (s = accept(l, (struct sockaddr*)&addr, &len)))
		goto fail3;

	closesocket(l);
	return tofds_(s, c, sv);

 fail3:
	closesocket(c);
 fail2:
	closesocket(l);
 fail1:
    aug_maperror(WSAGetLastError());
	return -1;
}

static int
dgrampair_(int protocol, int sv[2])
{
	socklen_t len;
	SOCKADDR_IN addr = { 0 };
	SOCKET c, s;

	if (INVALID_SOCKET == (s = socket(AF_INET, SOCK_DGRAM, protocol)))
		goto fail1;

	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if (SOCKET_ERROR == bind(s, (struct sockaddr*)&addr, sizeof(addr)))
		goto fail2;

	len = sizeof(addr);
	if (SOCKET_ERROR == getsockname(s, (struct sockaddr*)&addr, &len))
		goto fail2;

	if (INVALID_SOCKET == (c = socket(AF_INET, SOCK_DGRAM, protocol)))
		goto fail2;

	if (SOCKET_ERROR == connect(c, (struct sockaddr*)&addr, sizeof(addr)))
		goto fail3;

	return tofds_(s, c, sv);

 fail3:
	closesocket(c);
 fail2:
	closesocket(s);
 fail1:
    aug_maperror(WSAGetLastError());
	return -1;
}

AUGSYS_API int
aug_socket(int domain, int type, int protocol)
{
    SOCKET h = socket(domain, type, protocol);
    if (INVALID_SOCKET == h)
        return toerror_();

    return tofd_(h);
}

AUGSYS_API int
aug_accept(int s, struct sockaddr* addr, socklen_t* addrlen)
{
    SOCKET h = accept(_get_osfhandle(s), addr, addrlen);
    if (INVALID_SOCKET == h)
        return toerror_();

    return tofd_(h);
}

AUGSYS_API int
aug_bind(int s, const struct sockaddr* addr, socklen_t addrlen)
{
    if (SOCKET_ERROR == bind(_get_osfhandle(s), addr, addrlen))
        return toerror_();

    return 0;
}

AUGSYS_API int
aug_connect(int s, const struct sockaddr* addr, socklen_t addrlen)
{
    if (SOCKET_ERROR == connect(_get_osfhandle(s), addr, addrlen))
        return toerror_();

    return 0;
}

AUGSYS_API int
aug_getpeername(int s, struct sockaddr* addr, socklen_t* addrlen)
{
    if (SOCKET_ERROR == getpeername(_get_osfhandle(s), addr, addrlen))
        return toerror_();

    return 0;
}

AUGSYS_API int
aug_getsockname(int s, struct sockaddr* addr, socklen_t* addrlen)
{
    if (SOCKET_ERROR == getsockname(_get_osfhandle(s), addr, addrlen))
        return toerror_();

    return 0;
}

AUGSYS_API int
aug_listen(int s, int backlog)
{
    if (SOCKET_ERROR == listen(_get_osfhandle(s), backlog))
        return toerror_();

    return 0;
}

AUGSYS_API ssize_t
aug_recv(int s, void* buf, size_t len, int flags)
{
    if (SOCKET_ERROR == recv(_get_osfhandle(s), buf, (int)len, flags))
        return toerror_();

    return 0;
}

AUGSYS_API ssize_t
aug_recvfrom(int s, void* buf, size_t len, int flags, struct sockaddr* from,
             socklen_t* fromlen)
{
    if (SOCKET_ERROR == recvfrom(_get_osfhandle(s), buf, (int)len, flags, from,
                                 fromlen))
        return toerror_();

    return 0;
}

AUGSYS_API ssize_t
aug_send(int s, const void* buf, size_t len, int flags)
{
    if (SOCKET_ERROR == send(_get_osfhandle(s), buf, (int)len, flags))
        return toerror_();

    return 0;
}

AUGSYS_API ssize_t
aug_sendto(int s, const void* buf, size_t len, int flags,
           const struct sockaddr* to, socklen_t tolen)
{
    if (SOCKET_ERROR == sendto(_get_osfhandle(s), buf, (int)len, flags, to, tolen))
        return toerror_();

    return 0;
}

AUGSYS_API int
aug_getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen)
{
    if (SOCKET_ERROR == getsockopt(_get_osfhandle(s), level, optname, optval,
                                   optlen))
        return toerror_();

    return 0;
}

AUGSYS_API int
aug_setsockopt(int s, int level, int optname, const void* optval,
               socklen_t optlen)
{
    if (SOCKET_ERROR == setsockopt(_get_osfhandle(s), level, optname, optval,
                                   optlen))
        return toerror_();

    return 0;
}

AUGSYS_API int
aug_shutdown(int s, int how)
{
    if (SOCKET_ERROR == shutdown(_get_osfhandle(s), how))
        return toerror_();

    return 0;
}

AUGSYS_API int
aug_socketpair(int domain, int type, int protocol, int sv[2])
{
    if (AF_UNIX != domain) {
        aug_maperror(WSAEAFNOSUPPORT);
        return -1;
    }

	switch (type) {
	case SOCK_STREAM:
		return streampair_(protocol, sv);
	case SOCK_DGRAM:
		return dgrampair_(protocol, sv);
	}

	aug_maperror(WSAESOCKTNOSUPPORT);
	return -1;
}
