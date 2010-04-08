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
#define AUGNET_BUILD
#include "augnet/inet.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/socket.h"
#include "augsys/unistd.h" /* aug_close() */

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"
#include "augctx/string.h" /* aug_strlcpy() */

#if !defined(_WIN32)
# if HAVE_ALLOCA_H
#  include <alloca.h>
# endif /* HAVE_ALLOCA_H */
# include <netinet/tcp.h>
#else /* _WIN32 */
# include <malloc.h>       /* alloca() */
#endif /* _WIN32 */

#include <string.h>        /* strchr() */

AUGNET_API aug_sd
aug_tcpclient_BI(const char* host, const char* serv, struct aug_endpoint* ep)
{
    aug_sd sd;
    struct addrinfo hints, * res, * save;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (aug_getaddrinfo(host, serv, &hints, &res) < 0)
        return AUG_BADSD;

    save = res;

    do {
        sd = aug_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (AUG_BADSD == sd)
            continue; /* Error, try next. */

        aug_getendpoint(res, ep);

        /* EXCEPT: aug_tcpclient_BI -> aug_connect_BI; */

        if (0 <= aug_connect_BI(sd, ep))
            break; /* Success. */

        /* Try next if aug_connect() failed. */

        aug_sclose(sd);
        sd = AUG_BADSD;

    } while ((res = res->ai_next));

    aug_destroyaddrinfo(save);
    return sd;
}

AUGNET_API aug_sd
aug_tcpserver_N(const char* host, const char* serv, struct aug_endpoint* ep)
{
    aug_sd sd;
    struct addrinfo hints, * res, * save;

    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (aug_getaddrinfo(host, serv, &hints, &res) < 0)
        return AUG_BADSD;

    save = res;

    do {
        sd = aug_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (AUG_BADSD == sd)
            continue; /* Error, try next. */

        aug_getendpoint(res, ep);

        /* EXCEPT: aug_tcpserver_N -> aug_bind_N; */

        if (0 <= aug_setreuseaddr(sd, AUG_TRUE)
            && 0 <= aug_bind_N(sd, ep)
            && 0 <= aug_listen(sd, SOMAXCONN))
            break; /* Success. */

        /* Try next if failed. */

        aug_sclose(sd);
        sd = AUG_BADSD;

    } while ((res = res->ai_next));

    aug_destroyaddrinfo(save);
    return sd;
}

AUGNET_API aug_sd
aug_udpclient_BI(const char* host, const char* serv, struct aug_endpoint* ep,
                 aug_bool connect)
{
    aug_sd sd;
    struct addrinfo hints, * res, * save;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (aug_getaddrinfo(host, serv, &hints, &res) < 0)
        return AUG_BADSD;

    save = res;

    do {
        sd = aug_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (AUG_BADSD == sd)
            continue; /* Error, try next. */

        aug_getendpoint(res, ep);

        /* EXCEPT: aug_udpclient_BI -> aug_connect_BI; */

        if (!connect || 0 <= aug_connect_BI(sd, ep))
            break; /* Success. */

        /* Try next if aug_connect() failed. */

        aug_sclose(sd);
        sd = AUG_BADSD;

    } while ((res = res->ai_next));

    aug_destroyaddrinfo(save);
    return sd;
}

AUGNET_API aug_sd
aug_udpserver_N(const char* host, const char* serv, struct aug_endpoint* ep)
{
    aug_sd sd;
    struct addrinfo hints, * res, * save;

    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if (aug_getaddrinfo(host, serv, &hints, &res) < 0)
        return AUG_BADSD;

    save = res;

    do {
        sd = aug_socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (AUG_BADSD == sd)
            continue; /* Error, try next. */

        aug_getendpoint(res, ep);

        /* EXCEPT: aug_udpserver_N -> aug_bind_N; */

        if (0 <= aug_bind_N(sd, ep))
            break; /* Success. */

        /* Try next if aug_bind() failed. */

        aug_sclose(sd);
        sd = AUG_BADSD;

    } while ((res = res->ai_next));

    aug_destroyaddrinfo(save);
    return sd;
}

AUGNET_API struct aug_hostserv*
aug_parsehostserv(const char* src, struct aug_hostserv* dst)
{
    size_t len;
    char* serv;

    aug_strlcpy(dst->data_, src, sizeof(dst->data_));

    /* Locate host and serv separator. */

    if (!(serv = strrchr(dst->data_, ':'))) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("missing separator [%s]"), src);
        return NULL;
    }

    /* Calculate length of host part. */

    len = serv - dst->data_;

    /* Ensure host and serv parts exists. */

    if (!len) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("missing host part [%s]"), src);
        return NULL;
    }

    if ('\0' == *++serv) {
        aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                        AUG_MSG("missing service part [%s]"), src);
        return NULL;
    }

    /* The host part of an ipv6 address may be contained within square
       brackets. */

    if ('[' == dst->data_[0]) {

        if (']' != dst->data_[len - 1]) {
            aug_setctxerror(aug_tlx, __FILE__, __LINE__, "aug", AUG_EPARSE,
                            AUG_MSG("unmatched brackets [%s]"), src);
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

/* FIXME: switch to aug_bool. */

AUGNET_API aug_result
aug_setnodelay(aug_sd sd, int on)
{
    return aug_setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
}

AUGNET_API aug_result
aug_established(aug_sd sd)
{
    struct aug_endpoint ep;

    if (!aug_getpeername(sd, &ep)) {

        if (ENOTCONN != aug_errno(aug_tlerr))
            return -1;

        /* ENOTCONN */

        aug_setexcept(aug_tlx, AUG_EXNONE);
        return -1;
    }
    return 0;
}
