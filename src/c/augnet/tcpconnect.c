/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/tcpconnect.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augnet/inet.h"    /* aug_established() */

#include "augsys/unistd.h"  /* aug_close() */
#include "augsys/utility.h" /* aug_setnonblock() */

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"
#include "augctx/string.h"  /* bzero() */

#include <assert.h>

struct aug_tcpconnect_ {
    aug_mpool* mpool_;
    struct addrinfo* res_, *save_;
    aug_sd sd_;
};

AUGNET_API aug_tcpconnect_t
aug_createtcpconnect(aug_mpool* mpool, const char* host, const char* serv)
{
    aug_tcpconnect_t conn;
    struct addrinfo hints, * res;

    bzero(&hints, sizeof(hints));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if (AUG_ISFAIL(aug_getaddrinfo(host, serv, &hints, &res)))
        return NULL;

    if (!(conn  = aug_allocmem(mpool, sizeof(struct aug_tcpconnect_)))) {
        aug_destroyaddrinfo(res);
        return NULL;
    }

    conn->mpool_ = mpool;
    conn->res_ = conn->save_ = res;
    conn->sd_ = AUG_BADSD;

    aug_retain(mpool);
    return conn;
}

AUGNET_API void
aug_destroytcpconnect(aug_tcpconnect_t conn)
{
    aug_mpool* mpool = conn->mpool_;
    if (AUG_BADSD != conn->sd_)
        aug_sclose(conn->sd_);

    aug_destroyaddrinfo(conn->save_);
    aug_freemem(mpool, conn);
    aug_release(mpool);
}

AUGNET_API aug_sd
aug_tryconnect(aug_tcpconnect_t conn, struct aug_endpoint* ep, int* est)
{
    /* FIXME: needs review against assumptions in object.c. */

    aug_sd sd = conn->sd_;
    conn->sd_ = AUG_BADSD;

    /* Handle case where user has called after all connection attempts have
       failed. */

    if (!conn->res_) {
        assert(AUG_BADSD == sd);
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("address-list exhausted"));
        return AUG_BADSD;
    }

    /* Not on initial attempt. */

    if (AUG_BADSD != sd) {

        /* Check status of pending connection. */

        aug_result result = aug_established(sd);

        if (AUG_ISFAIL(result)) {

            if (AUG_ISNONE(result)) {

                /* Not established. */

                if ((conn->res_ = conn->res_->ai_next)) {

                    /* Try next address. */

                    if (AUG_ISFAIL(aug_sclose(sd)))
                        return AUG_BADSD;

                } else {

                    /* No more addresses: set error to connection failure
                       reason. */

                    aug_setsockerrinfo(aug_tlerr, __FILE__, __LINE__, sd);
                    aug_sclose(sd); /* May set errno */
                    return AUG_BADSD;
                }

            } else {


                /* Error testing for establishment. */

                aug_sclose(sd);
                return AUG_BADSD;
            }

        } else {

            /* Established. */

            aug_getendpoint(conn->res_, ep);
            goto done;
        }
    }

    /* Previous socket closed at this point. */

    do {

        int err;

        /* Create socket for next address. */

        sd = aug_socket(conn->res_->ai_family, conn->res_->ai_socktype,
                        conn->res_->ai_protocol);
        if (AUG_BADSD == sd)
            continue; /* Ignore this one. */

        if (AUG_ISFAIL(aug_ssetnonblock(sd, AUG_TRUE))) {
            aug_sclose(sd);
            return AUG_BADSD;
        }

        aug_getendpoint(conn->res_, ep);

        if (AUG_ISSUCCESS(aug_connect(sd, ep))) {

            /* Immediate establishment. */

            goto done;
        }

        err = aug_errno(aug_tlerr);

        if (EINPROGRESS == err || EWOULDBLOCK == err) {

            /* Connection pending. */

            *est = 0;
            return conn->sd_ = sd;
        }

        /* Failed for other reason. */

        if (AUG_ISFAIL(aug_sclose(sd))) /* Ignore this one. */
            return AUG_BADSD;

    } while ((conn->res_ = conn->res_->ai_next));

    /* Error set from last aug_connect() attempt. */

    return AUG_BADSD;

 done:

    *est = 1;
    return sd;
}
