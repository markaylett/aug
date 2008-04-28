/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/unistd.h"

#include <string.h> /* memcpy() */

AUGSYS_API aug_result
aug_sclose(aug_sd sd)
{
    return aug_fclose(sd);
}

AUGSYS_API aug_result
aug_ssetnonblock(aug_sd sd, aug_bool on)
{
    return aug_fsetnonblock(sd, on);
}

AUGSYS_API int
aug_socket(int domain, int type, int protocol)
{
    int fd = socket(domain, type, protocol);
    if (-1 == fd) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return -1;
    }

    return fd;
}

AUGSYS_API int
aug_accept(aug_sd sd, struct aug_endpoint* ep)
{
    int fd;
    ep->len_ = AUG_MAXADDRLEN;
    fd = accept(s,  &ep->un_.sa_, &ep->len_);
    if (-1 == fd) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    return tofd_(fd);
}

AUGSYS_API int
aug_bind(aug_sd sd, const struct aug_endpoint* ep)
{
    if (-1 == bind(s, &ep->un_.sa_, ep->len_)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_connect(aug_sd sd, const struct aug_endpoint* ep)
{
    if (-1 == connect(s, &ep->un_.sa_, ep->len_)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API struct aug_endpoint*
aug_getpeername(aug_sd sd, struct aug_endpoint* ep)
{
    ep->len_ = AUG_MAXADDRLEN;
    if (-1 == getpeername(s, &ep->un_.sa_, &ep->len_)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return NULL;
    }

    return ep;
}

AUGSYS_API struct aug_endpoint*
aug_getsockname(aug_sd sd, struct aug_endpoint* ep)
{
    ep->len_ = AUG_MAXADDRLEN;
    if (-1 == getsockname(s, &ep->un_.sa_, &ep->len_)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return NULL;
    }

    return ep;
}

AUGSYS_API int
aug_listen(aug_sd sd, int backlog)
{
    if (-1 == listen(s, backlog)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API ssize_t
aug_recv(aug_sd sd, void* buf, size_t len, int flags)
{
    ssize_t ret;
    if (-1 == (ret = recv(s, buf, len, flags)))
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);

    return ret;
}

AUGSYS_API ssize_t
aug_recvfrom(aug_sd sd, void* buf, size_t len, int flags,
             struct aug_endpoint* ep)
{
    ssize_t ret;
    ep->len_ = AUG_MAXADDRLEN;
    if (-1 == (ret = recvfrom(s, buf, len, flags, &ep->un_.sa_, &ep->len_)))
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);

    return ret;
}

AUGSYS_API ssize_t
aug_send(aug_sd sd, const void* buf, size_t len, int flags)
{
    ssize_t ret;
    if (-1 == (ret = send(s, buf, len, flags)))
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);

    return ret;
}

AUGSYS_API ssize_t
aug_sendto(aug_sd sd, const void* buf, size_t len, int flags,
           const struct aug_endpoint* ep)
{
    ssize_t ret;
    if (-1 == (ret = sendto(s, buf, len, flags, &ep->un_.sa_, ep->len_)))
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);

    return ret;
}

AUGSYS_API ssize_t
aug_sread(aug_sd sd, void* buf, size_t len)
{
    return aug_fread(sd, buf, len);
}

AUGSYS_API ssize_t
aug_sreadv(aug_sd sd, const struct iovec* iov, int size)
{
    return aug_freadv(sd, iov, size);
}

AUGSYS_API ssize_t
aug_swrite(aug_sd sd, const void* buf, size_t len)
{
    return aug_fwrite(sd, buf, len);
}

AUGSYS_API ssize_t
aug_swritev(aug_sd sd, const struct iovec* iov, int size)
{
    return aug_fwritev(sd, iov, size);
}

AUGSYS_API int
aug_getsockopt(aug_sd sd, int level, int optname, void* optval,
               socklen_t* optlen)
{
    int ret = getsockopt(s, level, optname, optval, optlen);
    if (SOL_SOCKET == level && SO_ERROR == optname) {

        /* Handle broken SO_ERROR semantics documented in Stevens. */

        switch (ret) {
        case -1:
            *((int*)optval) = errno;
            errno = 0;
            break;
        case 0:
            /* Correct semantics: error stored in optval. */
            break;
        default:
            *((int*)optval) = ret;
            break;
        }

    } else {

        if (-1 == ret) {
            aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
            return -1;
        }
    }

    return 0;
}

AUGSYS_API int
aug_setsockopt(aug_sd sd, int level, int optname, const void* optval,
               socklen_t optlen)
{
    if (-1 == setsockopt(s, level, optname, optval, optlen)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_shutdown(aug_sd sd, int how)
{
    if (-1 == shutdown(s, how)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

AUGSYS_API int
aug_socketpair(int domain, int type, int protocol, aug_sd sv[2])
{
    if (-1 == socketpair(domain, type, protocol, sv)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    if (-1 == aug_openfds(sv, aug_posixfdtype())) {
        close(sv[0]);
        close(sv[1]);
        return -1;
    }

    return 0;
}


AUGSYS_API char*
aug_inetntop(const struct aug_inetaddr* src, char* dst, socklen_t len)
{
    const char* ret = inet_ntop(src->family_, &src->un_, dst, len);
    if (!ret) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return NULL;
    }
    return dst;
}

AUGSYS_API struct aug_inetaddr*
aug_inetpton(int af, const char* src, struct aug_inetaddr* dst)
{
    int ret = inet_pton(af, src, &dst->un_);
    if (ret < 0) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return NULL;
    } else if (0 == ret) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid address: %s"), src);
        return NULL;
    }

    dst->family_ = af;
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
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCPOSIX, ret,
                       gai_strerror(ret));
        return -1;
    }
    return 0;
}

AUGSYS_API int
aug_getfamily(aug_sd sd)
{
    struct aug_endpoint ep;
    if (!aug_getsockname(s, &ep))
        return -1;

    return ep.un_.family_;
}
