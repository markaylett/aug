/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#include "augsys/uio.h" /* struct iovec */

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <io.h>
#include <malloc.h>     /* _alloca() */

static aug_result
streampair_(int protocol, aug_sd sv[2])
{
	socklen_t len;
	SOCKADDR_IN addr = { 0 };
	SOCKET l, c, s;

	if (INVALID_SOCKET == (l = socket(AF_INET, SOCK_STREAM, protocol)))
        goto fail1;

	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    /* SYSCALL: bind */
	if (SOCKET_ERROR == bind(l, (struct sockaddr*)&addr, sizeof(addr)))
		goto fail2;

	if (SOCKET_ERROR == listen(l, 1))
		goto fail2;

	len = sizeof(addr);
	if (SOCKET_ERROR == getsockname(l, (struct sockaddr*)&addr, &len))
		goto fail2;

	if (INVALID_SOCKET == (c = socket(AF_INET, SOCK_STREAM, protocol)))
		goto fail2;

    /* SYSCALL: connect */
	if (SOCKET_ERROR == connect(c, (struct sockaddr*)&addr, sizeof(addr)))
		goto fail3;

	len = sizeof(addr);
    /* SYSCALL: accept */
	if (INVALID_SOCKET == (s = accept(l, (struct sockaddr*)&addr, &len)))
		goto fail3;

	closesocket(l);
    sv[0] = s;
    sv[1] = c;
	return 0;

 fail3:
	closesocket(c);
 fail2:
	closesocket(l);
 fail1:
    aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
    return -1;
}

static aug_result
dgrampair_(int protocol, aug_sd sv[2])
{
	socklen_t len;
	SOCKADDR_IN addr = { 0 };
	SOCKET c, s;

	if (INVALID_SOCKET == (s = socket(AF_INET, SOCK_DGRAM, protocol)))
		goto fail1;

	addr.sin_family = AF_INET;
	addr.sin_port = 0;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    /* SYSCALL: bind */
	if (SOCKET_ERROR == bind(s, (struct sockaddr*)&addr, sizeof(addr)))
		goto fail2;

	len = sizeof(addr);
	if (SOCKET_ERROR == getsockname(s, (struct sockaddr*)&addr, &len))
		goto fail2;

	if (INVALID_SOCKET == (c = socket(AF_INET, SOCK_DGRAM, protocol)))
		goto fail2;

    /* SYSCALL: connect */
	if (SOCKET_ERROR == connect(c, (struct sockaddr*)&addr, sizeof(addr)))
		goto fail3;

    sv[0] = s;
    sv[1] = c;
	return 0;

 fail3:
	closesocket(c);
 fail2:
	closesocket(s);
 fail1:
    aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
    return -1;
}

AUGSYS_API aug_result
aug_sclose(aug_sd sd)
{
    if (SOCKET_ERROR == closesocket(sd)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }
    return 0;
}

AUGSYS_API aug_result
aug_ssetnonblock(aug_sd sd, aug_bool on)
{
    unsigned long arg = on ? 1 : 0;

    if (SOCKET_ERROR == ioctlsocket(sd, FIONBIO, &arg)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }

    return 0;
}

AUGSYS_API aug_sd
aug_socket(int domain, int type, int protocol)
{
    SOCKET h = socket(domain, type, protocol);
    if (INVALID_SOCKET == h)
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
    return h;
}

AUGSYS_API aug_sd
aug_accept(aug_sd sd, struct aug_endpoint* ep)
{
    SOCKET h;
    ep->len_ = AUG_MAXADDRLEN;
    /* SYSCALL: accept */
    if (INVALID_SOCKET == (h = accept(sd, &ep->un_.sa_, &ep->len_)))
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
    return h;
}

AUGSYS_API aug_result
aug_bind(aug_sd sd, const struct aug_endpoint* ep)
{
    /* SYSCALL: bind */
    if (SOCKET_ERROR == bind(sd, &ep->un_.sa_, ep->len_)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }
    return 0;
}

AUGSYS_API aug_result
aug_connect(aug_sd sd, const struct aug_endpoint* ep)
{
    /* SYSCALL: connect */
    if (SOCKET_ERROR == connect(sd, &ep->un_.sa_, ep->len_)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }
    return 0;
}

AUGSYS_API struct aug_endpoint*
aug_getpeername(aug_sd sd, struct aug_endpoint* ep)
{
    ep->len_ = AUG_MAXADDRLEN;
    if (SOCKET_ERROR == getpeername(sd, &ep->un_.sa_, &ep->len_)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return NULL;
    }
    return ep;
}

AUGSYS_API struct aug_endpoint*
aug_getsockname(aug_sd sd, struct aug_endpoint* ep)
{
    ep->len_ = AUG_MAXADDRLEN;
    if (SOCKET_ERROR == getsockname(sd, &ep->un_.sa_, &ep->len_)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return NULL;
    }
    return ep;
}

AUGSYS_API aug_result
aug_listen(aug_sd sd, int backlog)
{
    if (SOCKET_ERROR == listen(sd, backlog)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }
    return 0;
}

AUGSYS_API aug_rsize
aug_recv(aug_sd sd, void* buf, size_t len, int flags)
{
    /* SYSCALL: recv */
    ssize_t ret = recv(sd, buf, (int)len, flags);
    if (SOCKET_ERROR == ret) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }
    return ret;
}

AUGSYS_API aug_rsize
aug_recvfrom(aug_sd sd, void* buf, size_t len, int flags,
             struct aug_endpoint* ep)
{
    ssize_t ret;
    ep->len_ = AUG_MAXADDRLEN;

    /* SYSCALL: recvfrom */
    ret = recvfrom(sd, buf, (int)len, flags, &ep->un_.sa_, &ep->len_);
    if (SOCKET_ERROR == ret) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }
    return ret;
}

AUGSYS_API aug_rsize
aug_send(aug_sd sd, const void* buf, size_t len, int flags)
{
    /* SYSCALL: send */
    ssize_t ret = send(sd, buf, (int)len, flags);
    if (SOCKET_ERROR == ret) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }
    return ret;
}

AUGSYS_API aug_rsize
aug_sendto(aug_sd sd, const void* buf, size_t len, int flags,
           const struct aug_endpoint* ep)
{
    ssize_t ret = sendto(sd, buf, (int)len, flags, &ep->un_.sa_, ep->len_);
    if (SOCKET_ERROR == ret) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }
    return ret;
}

AUGSYS_API aug_rsize
aug_sread(aug_sd sd, void* buf, size_t len)
{
    return aug_recv(sd, buf, len, 0);
}

AUGSYS_API aug_rsize
aug_sreadv(aug_sd sd, const struct iovec* iov, int size)
{
    WSABUF* buf;
    int i;
    DWORD ret;

#if defined(_MSC_VER)
	__try {
#endif /* _MSC_VER */
        buf = _alloca(sizeof(WSABUF) * size);
#if defined(_MSC_VER)
	} __except (STATUS_STACK_OVERFLOW == GetExceptionCode()) {
		aug_setwin32error(aug_tlx, __FILE__, __LINE__,
                          ERROR_NOT_ENOUGH_MEMORY);
        return -1;
	}
#endif /* _MSC_VER */

    for (i = 0; i < size; ++i) {
        buf[i].len = iov[i].iov_len;
        buf[i].buf = iov[i].iov_base;
    }

    if (SOCKET_ERROR == WSARecv(sd, buf, size, &ret, 0, NULL, NULL)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }

    return (ssize_t)ret;
}

AUGSYS_API aug_rsize
aug_swrite(aug_sd sd, const void* buf, size_t len)
{
    return aug_send(sd, buf, len, 0);
}

AUGSYS_API aug_rsize
aug_swritev(aug_sd sd, const struct iovec* iov, int size)
{
    WSABUF* buf;
    int i;
    DWORD ret;

#if defined(_MSC_VER)
	__try {
#endif /* _MSC_VER */
        buf = _alloca(sizeof(WSABUF) * size);
#if defined(_MSC_VER)
	} __except (STATUS_STACK_OVERFLOW == GetExceptionCode()) {
		aug_setwin32error(aug_tlx, __FILE__, __LINE__,
                          ERROR_NOT_ENOUGH_MEMORY);
        return -1;
	}
#endif /* _MSC_VER */

    for (i = 0; i < size; ++i) {
        buf[i].len = iov[i].iov_len;
        buf[i].buf = iov[i].iov_base;
    }

    if (SOCKET_ERROR == WSASend(sd, buf, size, &ret, 0, NULL, NULL)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }

    return (ssize_t)ret;
}

AUGSYS_API aug_result
aug_getsockopt(aug_sd sd, int level, int optname, void* optval,
               socklen_t* optlen)
{
    int ret = getsockopt(sd, level, optname, optval, optlen);

    if (SOL_SOCKET == level && SO_ERROR == optname) {

        /* MSDN confirms that optval should be type int for SO_ERROR. */

        int* err = optval;

        /* If getsockopt() call failed. */

        if (SOCKET_ERROR == ret) {
            *err = WSAGetLastError();
            WSASetLastError(0);
        }

        /* Map Winsock error to Posix error. */

        *err = aug_win32posix(*err);

    } else if (SOCKET_ERROR == ret) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }
    return 0;
}

AUGSYS_API aug_result
aug_setsockopt(aug_sd sd, int level, int optname, const void* optval,
               socklen_t optlen)
{
    if (SOCKET_ERROR == setsockopt(sd, level, optname, optval, optlen)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }
    return 0;
}

AUGSYS_API aug_result
aug_sshutdown(aug_sd sd, int how)
{
    if (SOCKET_ERROR == shutdown(sd, how)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }
    return 0;
}

AUGSYS_API aug_result
aug_socketpair(int domain, int type, int protocol, aug_sd sv[2])
{
    if (AF_UNIX != domain) {
        SETAFNOSUPPORT_();
        return -1;
    }

	switch (type) {
	case SOCK_STREAM:
		return streampair_(protocol, sv);
	case SOCK_DGRAM:
		return dgrampair_(protocol, sv);
	}

    aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAESOCKTNOSUPPORT);
    return -1;
}

AUGSYS_API char*
aug_inetntop(const struct aug_inetaddr* src, char* dst, socklen_t len)
{
    DWORD dstlen = len;
    DWORD srclen;
    union {
        short family_;
        struct sockaddr sa_;
        struct sockaddr_in ipv4_;
#if HAVE_IPV6
        struct sockaddr_in6 ipv6_;
#endif /* HAVE_IPV6 */
    } un;
    bzero(&un, sizeof(un));

    switch (src->family_) {
    case AF_INET:
        un.family_ = AF_INET;
        un.ipv4_.sin_addr.s_addr = src->un_.ipv4_.s_addr;
		srclen = sizeof(un.ipv4_);
        break;
#if HAVE_IPV6
    case AF_INET6:
        un.family_ = AF_INET6;
        memcpy(&un.ipv6_.sin6_addr, &src->un_, sizeof(src->un_.ipv6_));
		srclen = sizeof(un.ipv6_);
        break;
#endif /* HAVE_IPV6 */
    default:
        SETAFNOSUPPORT_();
        return NULL;
    }

    if (SOCKET_ERROR == WSAAddressToString(&un.sa_, srclen, NULL, dst,
                                           &dstlen)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return NULL;
    }

    return dst;
}

AUGSYS_API struct aug_inetaddr*
aug_inetpton(int af, const char* src, struct aug_inetaddr* dst)
{
    INT len;
    union {
        short family_;
        struct sockaddr sa_;
        struct sockaddr_in ipv4_;
#if HAVE_IPV6
        struct sockaddr_in6 ipv6_;
#endif /* HAVE_IPV6 */
    } un;
    bzero(&un, sizeof(un));

    switch (af) {
    case AF_INET:
		len = sizeof(un.ipv4_);
        un.family_ = AF_INET;
        break;
#if HAVE_IPV6
    case AF_INET6:
		len = sizeof(un.ipv6_);
        un.family_ = AF_INET6;
        break;
#endif /* HAVE_IPV6 */
    default:
        SETAFNOSUPPORT_();
        return NULL;
    }

    if (SOCKET_ERROR == WSAStringToAddress((LPTSTR)src, af, NULL, &un.sa_,
                                           &len)) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, WSAGetLastError());
        return NULL;
    }

    if (AF_INET == (dst->family_ = af))
        dst->un_.ipv4_.s_addr = un.ipv4_.sin_addr.s_addr;
#if HAVE_IPV6
    else
        memcpy(&dst->un_, &un.ipv6_.sin6_addr, sizeof(dst->un_.ipv6_));
#endif /* HAVE_IPV6 */
    return dst;
}

AUGSYS_API void
aug_destroyaddrinfo(struct addrinfo* res)
{
    freeaddrinfo(res);
}

AUGSYS_API aug_result
aug_getaddrinfo(const char* host, const char* serv,
                const struct addrinfo* hints, struct addrinfo** res)
{
    int ret = getaddrinfo(host, serv, hints, res);
    if (0 != ret) {
        aug_setwin32error(aug_tlx, __FILE__, __LINE__, ret);
        return -1;
    }

    return 0;
}

AUGSYS_API aug_rint
aug_getfamily(aug_sd sd)
{
    WSAPROTOCOL_INFO info = { 0 };
    socklen_t len = sizeof(info);

    if (aug_getsockopt(sd, SOL_SOCKET, SO_PROTOCOL_INFO, &info, &len) < 0)
        return -1;

    return info.iAddressFamily;
}
