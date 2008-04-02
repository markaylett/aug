/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/base.h"
#include "augsys/errinfo.h"
#include "augsys/errno.h"
#include "augsys/log.h"
#include "augsys/uio.h"

#include <io.h>
#include <malloc.h> /* _alloca() */

static void
setbadfd_(aug_ctx* ctx, const char* file, int line)
{
    aug_seterrinfo(ctx, file, line, "aug", AUG_EINVAL,
                   AUG_MSG("invalid file descriptor"));
}

static int
close_(int fd)
{
    /* The _open_osfhandle() function allocates a C run-time file handle and
       sets it to point to the operating-system file handle.  When
       _open_osfhandle() function is used on a socket descriptor, both
       _close() and closesocket() should be called before exiting.  However,
       on Windows NT 4.0 Service Pack 3, closesocket() after _close() returns
       10038. */

    int ret = closesocket(_get_osfhandle(fd));
    if (SOCKET_ERROR == ret)
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());

#if defined(_MSC_VER)
    __try {
#endif /* _MSC_VER */
        close(fd);
#if defined(_MSC_VER)
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        AUG_DEBUG3("structured exception [%08x]", GetExceptionCode());
    }
#endif /* _MSC_VER */

    return SOCKET_ERROR == ret ? -1 : 0;
}

static ssize_t
read_(int fd, void* buf, size_t size)
{
    ssize_t ret = recv(_get_osfhandle(fd), buf, (int)size, 0);
    if (SOCKET_ERROR == ret)
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());

    return ret;
}

static ssize_t
readv_(int fd, const struct iovec* iov, int size)
{
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    WSABUF* buf;
    int i;
    DWORD ret;

#if defined(_MSC_VER)
	__try {
#endif /* _MSC_VER */
        buf = _alloca(sizeof(WSABUF) * size);
#if defined(_MSC_VER)
	} __except (STATUS_STACK_OVERFLOW == GetExceptionCode()) {
		aug_setwin32errinfo(NULL, __FILE__, __LINE__,
                            ERROR_NOT_ENOUGH_MEMORY);
		return -1;
	}
#endif /* _MSC_VER */

    for (i = 0; i < size; ++i) {
        buf[i].len = iov[i].iov_len;
        buf[i].buf = iov[i].iov_base;
    }

    if (SOCKET_ERROR == WSARecv((SOCKET)h, buf, size, &ret, 0, NULL, NULL)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }

    return (ssize_t)ret;
}

static ssize_t
write_(int fd, const void* buf, size_t len)
{
    ssize_t ret = send(_get_osfhandle(fd), buf, (int)len, 0);
    if (SOCKET_ERROR == ret)
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());

    return ret;
}

static ssize_t
writev_(int fd, const struct iovec* iov, int size)
{
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    WSABUF* buf;
    int i;
    DWORD ret;

#if defined(_MSC_VER)
	__try {
#endif /* _MSC_VER */
        buf = _alloca(sizeof(WSABUF) * size);
#if defined(_MSC_VER)
	}
	__except (STATUS_STACK_OVERFLOW == GetExceptionCode()) {
		aug_setwin32errinfo(NULL, __FILE__, __LINE__,
                            ERROR_NOT_ENOUGH_MEMORY);
		return -1;
	}
#endif /* _MSC_VER */

    for (i = 0; i < size; ++i) {
        buf[i].len = iov[i].iov_len;
        buf[i].buf = iov[i].iov_base;
    }

    if (SOCKET_ERROR == WSASend((SOCKET)h, buf, size, &ret, 0, NULL, NULL)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }

    return (ssize_t)ret;
}

static int
setnonblock_(int fd, int on)
{
    HANDLE h = (HANDLE)_get_osfhandle(fd);
    unsigned long arg = (unsigned long)on;

    if (SOCKET_ERROR == ioctlsocket((SOCKET)h, FIONBIO, &arg)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }

    return 0;
}

static struct aug_fdtype fdtype_ = {
    close_,
    read_,
    readv_,
    write_,
    writev_,
    setnonblock_
};

static int
tofd_(SOCKET h)
{
    int fd = _open_osfhandle(h, 0);
    if (-1 == fd) {
        setbadfd_(__FILE__, __LINE__);
        closesocket(h);
        return -1;
    }

    if (-1 == aug_openfd(fd, &fdtype_)) {
        close_(fd);
        return -1;
    }

    return fd;
}

static int
tofds_(SOCKET rd, SOCKET wr, int sv[2])
{
    int local[2];

    if (-1 == (local[0] = _open_osfhandle(rd, 0))) {
        setbadfd_(__FILE__, __LINE__);
        closesocket(rd);
        closesocket(wr);
        return -1;
    }

    if (-1 == (local[1] = _open_osfhandle(wr, 0))) {
        setbadfd_(__FILE__, __LINE__);
        close_(local[0]);
        closesocket(wr);
        return -1;
    }

    if (-1 == aug_openfds(local, &fdtype_)) {
        close_(local[0]);
        close_(local[1]);
        return -1;
    }

	sv[0] = local[0];
	sv[1] = local[1];
    return 0;
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
    aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
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
    aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
	return -1;
}

AUGSYS_API aug_fd
aug_socket(aug_ctx* ctx, int domain, int type, int protocol)
{
    SOCKET h = socket(domain, type, protocol);
    if (INVALID_SOCKET == h) {
        aug_setwin32errinfo(get_errinfo(ctx), __FILE__, __LINE__,
                            WSAGetLastError());
        return -1;
    }

    return tofd_(h);
}

AUGSYS_API int
aug_accept(int s, struct aug_endpoint* ep)
{
    SOCKET h;
    ep->len_ = AUG_MAXADDRLEN;
    h = accept(_get_osfhandle(s), &ep->un_.sa_, &ep->len_);
    if (INVALID_SOCKET == h) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }

    return tofd_(h);
}

AUGSYS_API int
aug_bind(int s, const struct aug_endpoint* ep)
{
    if (SOCKET_ERROR == bind(_get_osfhandle(s), &ep->un_.sa_, ep->len_)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_connect(int s, const struct aug_endpoint* ep)
{
    if (SOCKET_ERROR == connect(_get_osfhandle(s), &ep->un_.sa_, ep->len_)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }

    return 0;
}

AUGSYS_API struct aug_endpoint*
aug_getpeername(int s, struct aug_endpoint* ep)
{
    ep->len_ = AUG_MAXADDRLEN;
    if (SOCKET_ERROR == getpeername(_get_osfhandle(s), &ep->un_.sa_,
                                    &ep->len_)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
        return NULL;
    }

    return ep;
}

AUGSYS_API struct aug_endpoint*
aug_getsockname(int s, struct aug_endpoint* ep)
{
    ep->len_ = AUG_MAXADDRLEN;
    if (SOCKET_ERROR == getsockname(_get_osfhandle(s), &ep->un_.sa_,
                                    &ep->len_)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
        return NULL;
    }

    return ep;
}

AUGSYS_API int
aug_listen(int s, int backlog)
{
    if (SOCKET_ERROR == listen(_get_osfhandle(s), backlog)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }

    return 0;
}

AUGSYS_API ssize_t
aug_recv(int s, void* buf, size_t len, int flags)
{
    ssize_t ret = recv(_get_osfhandle(s), buf, (int)len, flags);
    if (SOCKET_ERROR == ret)
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());

    return ret;
}

AUGSYS_API ssize_t
aug_recvfrom(int s, void* buf, size_t len, int flags, struct aug_endpoint* ep)
{
    ssize_t ret;
    ep->len_ = AUG_MAXADDRLEN;
    ret = recvfrom(_get_osfhandle(s), buf, (int)len, flags, &ep->un_.sa_,
                   &ep->len_);
    if (SOCKET_ERROR == ret)
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());

    return ret;
}

AUGSYS_API ssize_t
aug_send(int s, const void* buf, size_t len, int flags)
{
    ssize_t ret = send(_get_osfhandle(s), buf, (int)len, flags);
    if (SOCKET_ERROR == ret)
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());

    return ret;
}

AUGSYS_API ssize_t
aug_sendto(int s, const void* buf, size_t len, int flags,
           const struct aug_endpoint* ep)
{
    ssize_t ret = sendto(_get_osfhandle(s), buf, (int)len, flags,
                         &ep->un_.sa_, ep->len_);
    if (SOCKET_ERROR == ret)
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());

    return ret;
}

AUGSYS_API int
aug_getsockopt(int s, int level, int optname, void* optval, socklen_t* optlen)
{
    if (SOCKET_ERROR == getsockopt(_get_osfhandle(s), level, optname, optval,
                                   optlen)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }

    /* Map Winsock error to Posix error. */

    if (SOL_SOCKET == level && SO_ERROR == optname) {
        int* err = optval;
        *err = aug_win32errno(*err);
    }

    return 0;
}

AUGSYS_API int
aug_setsockopt(int s, int level, int optname, const void* optval,
               socklen_t optlen)
{
    if (SOCKET_ERROR == setsockopt(_get_osfhandle(s), level, optname, optval,
                                   optlen)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_shutdown(int s, int how)
{
    if (SOCKET_ERROR == shutdown(_get_osfhandle(s), how)) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_socketpair(int domain, int type, int protocol, int sv[2])
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

	aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAESOCKTNOSUPPORT);
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
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
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
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, WSAGetLastError());
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

AUGSYS_API int
aug_getaddrinfo(const char* host, const char* serv,
                const struct addrinfo* hints, struct addrinfo** res)
{
    int ret = getaddrinfo(host, serv, hints, res);
    if (0 != ret) {
        aug_setwin32errinfo(NULL, __FILE__, __LINE__, ret);
        return -1;
    }
    return 0;
}

AUGSYS_API int
aug_getfamily(int s)
{
    WSAPROTOCOL_INFO info = { 0 };
    socklen_t len = sizeof(info);
    if (-1 == aug_getsockopt(s, SOL_SOCKET, SO_PROTOCOL_INFO, &info, &len))
        return -1;

    return info.iAddressFamily;
}
