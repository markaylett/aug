/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
#include "augsys/uio.h" /* aug_freadv() */
#include "augsys/unistd.h"

#include "augctx/base.h"
#include "augctx/errno.h"

#include <string.h>     /* memcpy() */

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

AUGSYS_API aug_sd
aug_accept(aug_sd sd, struct aug_endpoint* ep)
{
    int fd;
    ep->len_ = AUG_MAXADDRLEN;
    if (-1 == (fd = accept(sd,  &ep->un_.sa_, &ep->len_)))
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
    return fd;
}

AUGSYS_API aug_result
aug_bind(aug_sd sd, const struct aug_endpoint* ep)
{
    if (-1 == bind(sd, &ep->un_.sa_, ep->len_))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}

AUGSYS_API aug_result
aug_connect(aug_sd sd, const struct aug_endpoint* ep)
{
    if (-1 == connect(sd, &ep->un_.sa_, ep->len_))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}

AUGSYS_API struct aug_endpoint*
aug_getpeername(aug_sd sd, struct aug_endpoint* ep)
{
    ep->len_ = AUG_MAXADDRLEN;
    if (-1 == getpeername(sd, &ep->un_.sa_, &ep->len_)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return NULL;
    }

    return ep;
}

AUGSYS_API struct aug_endpoint*
aug_getsockname(aug_sd sd, struct aug_endpoint* ep)
{
    ep->len_ = AUG_MAXADDRLEN;
    if (-1 == getsockname(sd, &ep->un_.sa_, &ep->len_)) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return NULL;
    }

    return ep;
}

AUGSYS_API aug_result
aug_listen(aug_sd sd, int backlog)
{
    if (-1 == listen(sd, backlog))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}

AUGSYS_API aug_rsize
aug_recv(aug_sd sd, void* buf, size_t len, int flags)
{
    ssize_t ret = recv(sd, buf, len, flags);
    if (-1 == ret)
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_MKRESULT(ret);
}

AUGSYS_API aug_rsize
aug_recvfrom(aug_sd sd, void* buf, size_t len, int flags,
             struct aug_endpoint* ep)
{
    ssize_t ret;
    ep->len_ = AUG_MAXADDRLEN;
    if (-1 == (ret = recvfrom(sd, buf, len, flags, &ep->un_.sa_, &ep->len_)))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_MKRESULT(ret);
}

AUGSYS_API aug_rsize
aug_send(aug_sd sd, const void* buf, size_t len, int flags)
{
    ssize_t ret = send(sd, buf, len, flags);
    if (-1 == ret)
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_MKRESULT(ret);
}

AUGSYS_API aug_rsize
aug_sendto(aug_sd sd, const void* buf, size_t len, int flags,
           const struct aug_endpoint* ep)
{
    ssize_t ret = sendto(sd, buf, len, flags, &ep->un_.sa_, ep->len_);
    if (-1 == ret)
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_MKRESULT(ret);
}

AUGSYS_API aug_rsize
aug_sread(aug_sd sd, void* buf, size_t len)
{
    return aug_fread(sd, buf, len);
}

AUGSYS_API aug_rsize
aug_sreadv(aug_sd sd, const struct iovec* iov, int size)
{
    return aug_freadv(sd, iov, size);
}

AUGSYS_API aug_rsize
aug_swrite(aug_sd sd, const void* buf, size_t len)
{
    return aug_fwrite(sd, buf, len);
}

AUGSYS_API aug_rsize
aug_swritev(aug_sd sd, const struct iovec* iov, int size)
{
    return aug_fwritev(sd, iov, size);
}

AUGSYS_API aug_result
aug_getsockopt(aug_sd sd, int level, int optname, void* optval,
               socklen_t* optlen)
{
    int ret = getsockopt(sd, level, optname, optval, optlen);
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

    } else if (-1 == ret)
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}

AUGSYS_API aug_result
aug_setsockopt(aug_sd sd, int level, int optname, const void* optval,
               socklen_t optlen)
{
    if (-1 == setsockopt(sd, level, optname, optval, optlen))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}

AUGSYS_API aug_result
aug_sshutdown(aug_sd sd, int how)
{
    if (-1 == shutdown(sd, how))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}

AUGSYS_API aug_result
aug_socketpair(int domain, int type, int protocol, aug_sd sv[2])
{
    if (-1 == socketpair(domain, type, protocol, sv))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}


AUGSYS_API char*
aug_inetntop(const struct aug_inetaddr* src, char* dst, socklen_t len)
{
    const char* ret = inet_ntop(src->family_, &src->un_, dst, len);
    if (!ret) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return NULL;
    }
    return dst;
}

AUGSYS_API struct aug_inetaddr*
aug_inetpton(int af, const char* src, struct aug_inetaddr* dst)
{
    int ret = inet_pton(af, src, &dst->un_);
    if (ret < 0) {
        aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
        return NULL;
    } else if (0 == ret) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
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

AUGSYS_API aug_result
aug_getaddrinfo(const char* host, const char* serv,
                const struct addrinfo* hints, struct addrinfo** res)
{
    int ret = getaddrinfo(host, serv, hints, res);
    if (0 != ret) {
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "posix", ret,
                       gai_strerror(ret));
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS;
}

AUGSYS_API aug_rint
aug_getfamily(aug_sd sd)
{
    struct aug_endpoint ep;
    if (!aug_getsockname(sd, &ep))
        return AUG_FAILERROR;

    return AUG_MKRESULT(ep.un_.family_);
}
